using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using Xbim.Common;
using Xbim.Geometry.Abstractions;

namespace Xbim.Geometry.GeomService.UI
{
    /// <summary>
    /// Passes the meshing call to an external executable in a separate thread.
    /// This one passes the loglevl parameter as appropriate to the external exe.
    /// </summary>
    internal class ExternalExecutableMesher
    {
        private string? ExecutableFullPath = null;

        private const string ExeName = "Xbim.Geometry.GeomService.exe";

        public bool ExecutableFound => ExecutableFullPath != null;

        public ExternalExecutableMesher()
        {
            var t = System.Reflection.Assembly.GetExecutingAssembly().Location;
            var workingAssemblyFile = new FileInfo(t);
            if (!workingAssemblyFile.Exists || workingAssemblyFile.Directory is null)
                return;
            var joined = Path.Combine(workingAssemblyFile.Directory.FullName, ExeName);
            if (File.Exists(joined))
            {
                ExecutableFullPath = joined;

                // we can replace the config to help the process start
                var destConfig = ExecutableFullPath + ".config";
                var sourceConfig = workingAssemblyFile.FullName + ".config";
                if (File.Exists(sourceConfig))
                    File.Copy(sourceConfig, destConfig, true);
            }
        }

        public async Task<string> EnsureGeometryAsync(FileInfo f, bool adjustWcs, bool singleThread, XGeometryEngineVersion engineVer,
            CancellationToken cancellationToken = default, ReportProgressDelegate? progressDelegate = null, LogLevel logLevel = LogLevel.Debug)
        {
            if (ExecutableFullPath == null)
            {
                return "";
            }
            if (f.Extension.ToLowerInvariant() == ".ifc" || f.Extension.ToLowerInvariant() == ".ifczip")
            {
                // needs conversion
                return await ConvertItAsync(f, adjustWcs, singleThread, engineVer, cancellationToken, progressDelegate, logLevel);
            }
            return f.FullName;
        }

        public bool RequestLog { get; set; } = false;

        public int TimeOutMilliseconds { get; set; } = 30000; // default 30 seconds timeout

        ReportProgressDelegate? reportProgressUp;

        public IReadOnlyList<string> SummaryExecution => summaryExecution.AsReadOnly();

        private List<string> summaryExecution = new List<string>();

        int iIteration = 0;

        int iProcessId = -1;

        string fileProcessDump = "";

        public List<AttemptConfiguration> AttemptSequence { get; set; } = [
                (false, XGeometryEngineVersion.V6),
                (true, XGeometryEngineVersion.V6),
                (false, XGeometryEngineVersion.V5),
                (true, XGeometryEngineVersion.V5),
                ];

        private async Task<string> ConvertItAsync(FileInfo ifcfile, bool adjustWcs, bool singleThread, XGeometryEngineVersion engineVer,
            CancellationToken cancellationToken, ReportProgressDelegate? progressDelegate, LogLevel logLevel)
        {
            summaryExecution = [];
            reportProgressUp = progressDelegate;
            var xbimFileName = new FileInfo(Path.ChangeExtension(ifcfile.FullName, "xbim"));
            if (xbimFileName.Exists)
                File.Delete(xbimFileName.FullName);
            string infileParam =
                $"""
				"/in:{ifcfile.FullName}"
				""";

            fileProcessDump = Path.ChangeExtension(ifcfile.FullName, "dmp");

            // doing this wrapped in a task.run() to remove the "missing await" warning/error
            string adjustParam = await Task.Run(() => MakeParam("adjust", adjustWcs));
            iIteration = 0;
            foreach (var attempt in AttemptSequence)
            {
                iIteration++;
                string stParam = MakeParam("st", attempt.useSingleThread);
                string stEngine = MakeParam("eng", attempt.engineVer);
                string stLog = MakeParam("log", RequestLog);
                string stLogLevel = MakeParam("ll", logLevel);
                string stProg = MakeParam("progress", true);
                var thisArguments = string.Join(" ", [infileParam, adjustParam, stParam, stEngine, stLog, stProg, stLogLevel]);
                var tFullCommand = $"{ExecutableFullPath} {thisArguments}";
                Debug.WriteLine(tFullCommand);

                var process = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = ExecutableFullPath,
                        Arguments = thisArguments,
                        CreateNoWindow = true,
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                    }
                };
                process.OutputDataReceived += ProcessDataReceived;

                process.Start();
                process.BeginOutputReadLine();
                iProcessId = process.Id;

                bool isMainCompleted = false;
                try
                {
                    // we start two tasks, one performs the computation, another for timeout check
                    using var ctsTimeout = new CancellationTokenSource();
                    int timeoutMilliseconds = TimeOutMilliseconds;
                    var waitForExitTask = Task.Run(() => process.WaitForExit(), cancellationToken);
                    var timeoutTask = Task.Delay(timeoutMilliseconds, ctsTimeout.Token);
                    var completedTask = await Task.WhenAny(waitForExitTask, timeoutTask);

                    if (completedTask == timeoutTask)
                    {
                        // Timeout occurred, kill the process
                        if (!process.HasExited)
                        {
                            process.Kill();
                            await waitForExitTask; // Ensure process resources are released
                        }
                        summaryExecution.Add($"{attempt}, TIMEOUT");
                        continue;
                    }
                    else
                    {
                        // Process exited - no timeout, but possible crash
                        isMainCompleted = true;
                        ctsTimeout.Cancel(); // Cancel the timeout task
                    }
                }
                catch (Exception)
                {
                    if (!process.HasExited)
                    {
                        process.Kill();
                        process.WaitForExit();
                    }
                    if (!isMainCompleted)
                    {
                        iProcessId = -1;
                        throw;
                    }
                }

                if (cancellationToken.IsCancellationRequested)
                {
                    if (!process.HasExited)
                    {
                        process.Kill();
                    }
                    summaryExecution.Add($"{attempt}, CANCELLED");
                    iProcessId = -1;
                    return string.Empty;
                }
                var exitCode = (ExitCodes)process.ExitCode;
                
                var ret = $"{attempt}, {exitCode}, meshed";
                if (exitCode == ExitCodes.ExitCodeOK)
                {
                    summaryExecution.Add(ret);
                    iProcessId = -1;
                    return xbimFileName.FullName;
                }
                // check that the process is completed
                if (exitCode == ExitCodes.ExitCodeErrorCopying) // problem copying, we can try to wait a bit longer
                {
                    var s = Stopwatch.StartNew();
                    while (!IsFileReady(xbimFileName.FullName) && s.ElapsedMilliseconds < 5000)
                    {
                        Debug.WriteLine($"Waited {s.ElapsedMilliseconds}msec");
                        Thread.Sleep(1500);
                    }
                    if (IsFileReady(xbimFileName.FullName))
                    {
                        summaryExecution.Add(ret);
                        iProcessId = -1;
                        return xbimFileName.FullName;
                    }
                }
                summaryExecution.Add($"{attempt}, {exitCode}, CRASH");
            }
            summaryExecution.Add("TOTALCRASH");
            iProcessId = -1;
            return string.Empty;
        }

        private readonly Regex regExProgressString = new Regex(@"^(?<percent>[+-]?\d+)(?<description>.*)$", RegexOptions.Compiled);

        private void ProcessDataReceived(object sender, DataReceivedEventArgs args)
        {
            if (args.Data != null && reportProgressUp is not null)
            {
                var m = regExProgressString.Match(args.Data);
                if (m.Success)
                {
                    if (int.TryParse(m.Groups["percent"].Value, out var prog))
                    {
                        reportProgressUp?.Invoke(prog, $"{m.Groups["description"].Value.Trim()}{(iIteration > 1 ? $" ({iIteration})" : "")}");
                    }
                }
            }
        }

        static bool IsFileReady(string filename)
        {
            try
            {
                using FileStream inputStream = File.Open(filename, FileMode.Open, FileAccess.Read, FileShare.None);
                return inputStream.Length > 0;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private static string MakeParam(string paramName, object paramValue)
        {
            return $"/{paramName}:{paramValue}";
        }

        internal void RequestDumpStack()
        {
            if (iProcessId == -1)
                return;
            summaryExecution.Add($"Requesting process dump... {fileProcessDump}");
            Process.Start("werfault.exe", $"-u -p {iProcessId}");
        }
    }

    internal record struct AttemptConfiguration(bool useSingleThread, XGeometryEngineVersion engineVer)
    {
        public static implicit operator (bool useSingleThread, XGeometryEngineVersion engineVer)(AttemptConfiguration value)
        {
            return (value.useSingleThread, value.engineVer);
        }

        public static implicit operator AttemptConfiguration((bool useSingleThread, XGeometryEngineVersion engineVer) value)
        {
            return new AttemptConfiguration(value.useSingleThread, value.engineVer);
        }

        public override string ToString()
        {
            return $"{engineVer} {(useSingleThread ? "singleThread" : "multiThread")}";
        }
    }
}

