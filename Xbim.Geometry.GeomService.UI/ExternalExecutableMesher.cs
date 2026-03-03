using Humanizer;
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

        public async Task<string> EnsureGeometryAsync(
            FileInfo f, bool adjustWcs, CancellationToken cancellationToken = default,
            ReportProgressDelegate? progressDelegate = null, LogLevel logLevel = LogLevel.Debug,
            int maxMemMb = 1024)
        {
            if (ExecutableFullPath == null)
            {
                return "";
            }
            if (f.Extension.ToLowerInvariant() == ".ifc" || f.Extension.ToLowerInvariant() == ".ifczip")
            {
                // needs conversion
                return await ConvertItAsync(f, adjustWcs, cancellationToken, progressDelegate, logLevel, maxMemMb);
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

        private async Task<string> ConvertItAsync(FileInfo ifcfile, bool adjustWcs,
            CancellationToken cancellationToken, ReportProgressDelegate? progressDelegate, LogLevel logLevel, int maxMemMb)
        {
            var maxMemBytes = maxMemMb * 1024L * 1024L;
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
                    using var ctsControlTasksCancellation = new CancellationTokenSource();
                    int timeoutMilliseconds = TimeOutMilliseconds;
                    var waitForExitTask = Task.Run(() => process.WaitForExit(), cancellationToken);
                    var timeoutTask = Task.Delay(timeoutMilliseconds, ctsControlTasksCancellation.Token);
                    var memoryCheckTask = Task.Run(() =>
                    {
                        while (!process.HasExited)
                        {
                            try
                            {
                                process.Refresh();
                                DebugMem(process);
                                if (process.WorkingSet64 > maxMemBytes)
                                {
                                    summaryExecution.Add($"{attempt}, cancelling for memory limit, using {process.WorkingSet64.Bytes().Humanize()}");
                                    Debug.WriteLine($"Process {process.Id} is using too much memory: {process.WorkingSet64} bytes. Killing it.");
                                    process.Kill();
                                    break;
                                }
                            }
                            catch (Exception ex)
                            {
                                Debug.WriteLine($"Error checking memory usage for process {process.Id}: {ex.Message}");
                                break;
                            }
                            Thread.Sleep(1000); // Check every second
                        }
                    }, ctsControlTasksCancellation.Token);
                    var completedTask = await Task.WhenAny(waitForExitTask, timeoutTask, memoryCheckTask);

                    if (completedTask == waitForExitTask)
                    {
                        // Process exited - no timeout, but possible crash
                        isMainCompleted = true;
                        ctsControlTasksCancellation.Cancel(); // Cancel the timeout task and the memory check task
                    }
                    else
                    {
                        // stop occurred, kill the process
                        if (!process.HasExited)
                        {
                            process.Kill();
                            await waitForExitTask; // Ensure process resources are released
                        }
                        summaryExecution.Add($"{attempt}, CANCELLED");
                        continue;
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
                ExitCodes exitCode = ExitCodes.ExitCodeUndefinedError;
                try
                {
                    exitCode = (ExitCodes)process.ExitCode;
                }
                catch { }
                

                var ret = $"{attempt}, {exitCode}, meshed";
                if (exitCode == ExitCodes.ExitCodeOK)
                {
                    summaryExecution.Add(ret);
                    iProcessId = -1;
                    return xbimFileName.FullName;
                }
                else if (exitCode == ExitCodes.ExitCodeInvalidIFC)
                {
                    summaryExecution.Add($"INVALIDFILE PARSING {ifcfile.FullName}");
                    iProcessId = -1;
                    return string.Empty;
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
            summaryExecution.Add($"TOTALCRASH {ifcfile.FullName}");
            iProcessId = -1;
            return string.Empty;
        }

        private void DebugMem(Process process)
        {
            string[] t = [
                process.WorkingSet64.Bytes().Humanize(),
                process.PrivateMemorySize64.Bytes().Humanize(),
                process.VirtualMemorySize64.Bytes().Humanize()
                ];
            Debug.WriteLine($"mem: {t[0]}, prvt: {t[1]}, vrt: {t[2]}");
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

