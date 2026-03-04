using Humanizer;
using Microsoft.Extensions.FileSystemGlobbing;
using Microsoft.Extensions.FileSystemGlobbing.Abstractions;
using Microsoft.Extensions.Logging;
using Serilog;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Xbim.Geometry.GeomService.UI; // (if you put the extension in this namespace)

namespace Xbim.Geometry.GeomService.UI
{
    /// <summary>
    /// Frontend UI for the GeomService executable meshing service
    /// </summary>
    public partial class GeomServiceUI : Form
    {
        /// <summary>
        /// Default constructor providing UI Initialization
        /// </summary>
        public GeomServiceUI(string[] arguments)
        {
            InitializeComponent();
            txtSources.Text = @"C:\Data\Ifc\GeomRegressionTest.glob";
            if (arguments.Length > 0)
            {
                txtSources.Text = string.Join(" ", arguments);
            }
            _meshHelper = new ExternalExecutableMesher();
            AddLogEntry("Syntax of glob files:");
            AddLogEntry("**/*.ifc -> All ifc files, including subdirectories");
            AddLogEntry("SomeFolder/*.ifc -> All ifc files in SomeFolder, excluding subdirectories");
            AddLogEntry("SomeFolder/**/*.ifc -> All ifc files inside SomeFolder, including subdirectories");
            AddLogEntry("!SomeFolder/**/*.ifc -> Exclamation excludes the files");
            AddLogEntry("#!SomeFolder/**/*.ifc -> Lines starting with # are comments and are ignored");
            cmbLoggingLevel.SelectedItem = "Debug";
            foreach (var seq in _meshHelper.AttemptSequence)
            {
                var it = new ListViewItem(seq.ToString())
                {
                    Tag = seq,
                    Checked = true
                };
                lstSequence.Items.Add(it);
            }
            if (_meshHelper.ExecutableFound)
            {
                AddLogEntry("GeomService executable found and ready.");
            }
            else
            {
                AddLogEntry("GeomService executable NOT found. Please ensure Xbim.Geometry.GeomService.exe is in the same folder as this application.");
            }
        }

        private IEnumerable<FileInfo> GetIfcFiles(string text)
        {
            FileInfo? fi = null;
            try
            {
                fi = new FileInfo(txtSources.Text); // if it's a file then ok.
            }
            catch (Exception)
            {
            }
            if (fi is null)
            {
                var matcher = new Matcher();
                var t = Regex.Match(txtSources.Text, @"^(?<drive>[a-zA-Z]):\\(?<rest>.*)$");
                if (t.Success)
                {
                    matcher.AddInclude(t.Groups["rest"].Value);
                    var dir = new DirectoryInfo($"{t.Groups["drive"].Value}:\\");
                    var result = matcher.Execute(new DirectoryInfoWrapper(dir));
                    foreach (var file in result.Files)
                    {
                        var comb = Path.Combine(dir.FullName, file.Path);
                        FileInfo f = new FileInfo(comb);
                        yield return f;
                    }
                }
                yield break;
            }

            if (!fi.Exists)
                yield break;
            if (fi.Extension.ToLower() == ".ifc" || fi.Extension.ToLower() == ".ifczip")
            {
                // single file
                yield return fi;
            }
            else if (fi.Extension.ToLower() == ".glob")
            {
                if (fi.Directory is null)
                    yield break;
                var lines = File.ReadLines(txtSources.Text);
                var matcher = new Matcher();
                foreach (var line in lines)
                {
                    if (line.StartsWith("#") || string.IsNullOrWhiteSpace(line))
                        continue; // skip comments and empty lines
                    if (line.StartsWith("!"))
                    {
                        matcher.AddExclude(line.Substring(1).Trim());
                    }
                    else
                        matcher.AddInclude(line);
                }
                var result = matcher.Execute(new DirectoryInfoWrapper(fi.Directory));
                foreach (var file in result.Files)
                {
                    var comb = Path.Combine(fi.Directory.FullName, file.Path);
                    FileInfo f = new FileInfo(comb);
                    yield return f;
                }
            }
            else
            {
                DirectoryInfo d = new DirectoryInfo(txtSources.Text);
                if (d.Exists && fi.Directory is not null)
                {
                    var matcher = new Matcher();
                    matcher.AddInclude("**/*.ifc");
                    matcher.AddInclude("**/*.ifczip");
                    var result = matcher.Execute(new DirectoryInfoWrapper(d));
                    foreach (var file in result.Files)
                    {
                        var comb = Path.Combine(fi.Directory.FullName, file.Path);
                        FileInfo f = new FileInfo(comb);
                        yield return f;
                    }
                }
            }
        }

        private const int MaxLogEntries = 300;
        private readonly Queue<string> _logEntries = new Queue<string>();

        private const int MaxActionLogEntries = 20;
        private readonly Queue<string> _actionLogEntries = new Queue<string>();

        private readonly ExternalExecutableMesher _meshHelper;

        private void AddLogEntry(string message, StreamWriter? fileLogger = null)
        {
            // log to file if provided
            fileLogger?.WriteLine($"{DateTime.Now:T} {message}");
            fileLogger?.Flush();
            LogMessageOnListBox(message, _logEntries, listBoxLog, MaxLogEntries);
        }

        private static void LogMessageOnListBox(string message, Queue<string> queue, ListBox uiList, int maxCount)
        {
            if (queue.Count >= maxCount)
                queue.Dequeue(); // Remove oldest
            queue.Enqueue($"{DateTime.Now:T} {message}");

            // Update the ListBox on the UI thread
            uiList.BeginUpdate();
            uiList.Items.Clear();
            uiList.Items.AddRange(queue.ToArray());
            uiList.TopIndex = uiList.Items.Count - 1; // Scroll to last
            uiList.EndUpdate();
        }

        private void btnSearchGlob_Click(object sender, EventArgs e)
        {
            using var openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "Glob files (*.glob)|*.glob|ifc files (*.ifc)|*.ifc|All files (*.*)|*.*";
            openFileDialog.Title = "Select a .glob file";
            openFileDialog.Multiselect = false;
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                // Use the selected file path
                txtSources.Text = openFileDialog.FileName;
            }
        }

        CancellationTokenSource? _cts;

        bool firstLaunch = true;

        private async void cmdConvertGeometry_Click(object sender, EventArgs e)
        {
            _meshHelper.TimeOutMilliseconds = (int)nudTimeoutMinutes.Value * 60 * 1000;
            var memoryMax = (int)nudMemoryLimit.Value;
            var actions = new List<AttemptConfiguration>();
            foreach (ListViewItem item in lstSequence.Items)
            {
                if (item.Checked && item.Tag is AttemptConfiguration attc)
                {
                    actions.Add(attc);
                }
            }
            if (actions.Count == 0)
            {
                MessageBox.Show("Please select at least one action in the sequence list.", "No actions selected", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            _meshHelper.AttemptSequence = actions;
            if (firstLaunch)
            {
                _logEntries.Clear();
                firstLaunch = false;
            }
            var files = GetFiles();
            var tot = files.Count;
            if (tot == 0)
            {
                LogMessageOnListBox($"No files to process.", _logEntries, listBoxLog, MaxLogEntries);
                return;
            }
            var cnt = 0;
            string execLog = GetLogFilePath();
            LogMessageOnListBox($"Logging at {execLog}", _logEntries, listBoxLog, MaxLogEntries);
            var ll = GetLogLevel(cmbLoggingLevel.Text);

            cmdConvertGeometry.Enabled = false;
            cmdCancelConvertGeometry.Enabled = true;
            cmdRequestDump.Enabled = cmdCancelConvertGeometry.Enabled;
            _cts = new CancellationTokenSource();
            var tmpCursor = Cursor;
            try
            {
                Cursor = Cursors.WaitCursor;
                using var fileLogger = File.AppendText(execLog);
                AddLogEntry($"=== Log Started at {DateTime.Now:G}", fileLogger);
                AddLogEntry($"Processing {tot} files from {txtSources.Text}", fileLogger);
                foreach (var file in files)
                {
                    progFiles.Value = (int)((cnt / (double)tot) * 100);
                    Stopwatch sw = Stopwatch.StartNew();
                    var fileSize = file.Length.Bytes().Humanize();
                    AddLogEntry($"Processing file: {file.FullName}, {fileSize}", fileLogger);
                    _meshHelper.RequestLog = true;
                    var meshedFile = await _meshHelper.EnsureGeometryAsync(file, false, _cts.Token, ReportProgressInBar, ll, memoryMax);
                    ReportProgressInBar(0, "File completed");
                    foreach (var logEntry in _meshHelper.SummaryExecution)
                    {
                        AddLogEntry(logEntry, fileLogger);
                    }
                    if (!string.IsNullOrEmpty(meshedFile))
                    {
                        var meshedFileSize = new FileInfo(meshedFile).Length.Bytes().Humanize();
                        var sec = sw.Elapsed.TotalSeconds;
                        var speed = Convert.ToInt64(file.Length / sec).Bytes().Humanize();
                        var msg = $"Processed {fileSize} in {sec:F2} seconds. Meshed size: {meshedFileSize}, speed: {speed}/sec.";
                        AddLogEntry(msg);
                    }
                    cnt++;
                    if (_cts.IsCancellationRequested)
                        break;
                }
                progFiles.Value = 0;
                ReportProgressInBar(0, "Processing files completed");
                AddLogEntry("Processing files completed");
            }
            catch (OperationCanceledException)
            {
                AddLogEntry("Operation was canceled.");
            }
            catch (Exception ex)
            {
                AddLogEntry($"Error: {ex.Message}");
            }
            finally
            {
                _cts.Dispose();
                _cts = null;
                cmdConvertGeometry.Enabled = true;
                cmdCancelConvertGeometry.Enabled = false;
                cmdRequestDump.Enabled = cmdCancelConvertGeometry.Enabled;
                Cursor = tmpCursor;
            }
        }

        private List<FileInfo> GetFiles()
        {
            var files = GetIfcFiles(txtSources.Text).ToList();
            if (chkSkipMeshed.Checked)
            {
                // reducing files to those that do not have .xbim meshed files already present, to avoid unnecessary processing
                files = files.Where(f =>
                {
                    var rep = Path.ChangeExtension(f.FullName, ".xbim");
                    return (!File.Exists(rep)); // skip if .xbim exists for the file
                }).ToList();
            }

            return files;
        }

        private LogLevel GetLogLevel(string text)
        {
            return text switch
            {
                "Information" => LogLevel.Information,
                "Warning" => LogLevel.Warning,
                "Error" => LogLevel.Error,
                "Critical" => LogLevel.Critical,
                "Trace" => LogLevel.Trace,
                _ => LogLevel.Debug,
            };
        }

        private string GetLogFilePath()
        {
            var t = Path.ChangeExtension(txtSources.Text, ".log");
            try
            {
                var t2 = new FileInfo(t);
                return t;
            }
            catch (Exception)
            {
                var t3 = Guid.NewGuid();
                return Path.Combine(Path.GetTempPath(), $"{t3}.log");
            }
        }

        string lastUserState = string.Empty;
        private void ReportProgressInBar(int percentProgress, object userState)
        {
            if (percentProgress < 0 || percentProgress > 100)
                return;

            var userStateString = userState.ToString() ?? "";
            if (lastUserState != userStateString)
            {
                lastUserState = userStateString;
                actionLog.InvokeIfRequired(s =>
                {
                    LogMessageOnListBox(userStateString, _actionLogEntries, actionLog, MaxActionLogEntries);
                });
            }
            progSingleFile.InvokeIfRequired(
                s =>
                {
                    s.Value = percentProgress;
                });
        }

        private void button1_Click(object sender, EventArgs e)
        {
            var p = GetLogFilePath();
            if (!File.Exists(p))
                return;
            OpenWithDefaultApp(p);
        }

        /// <summary>
        /// Launch default application for the given file
        /// </summary>
        /// <param name="filePath"></param>
        public static void OpenWithDefaultApp(string filePath)
        {
            var process = new Process
            {
                StartInfo = new ProcessStartInfo(filePath)
                {
                    UseShellExecute = true
                }
            };
            process.Start();
        }

        private void cmdCancelConvertGeometry_Click(object sender, EventArgs e)
        {
            _cts?.Cancel();
            AddLogEntry("Cancellation Request logged");
        }

        private void listBoxLog_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var r = new Regex(@".*Processing file: (?<filename>.*)$", RegexOptions.Compiled);
            if (listBoxLog.SelectedItem is not null)
            {
                var str = listBoxLog.SelectedItem.ToString() ?? "";
                var t = r.Match(str);
                if (t.Success)
                {
                    var fileName = t.Groups["filename"].Value;
                    if (File.Exists(fileName))
                    {
                        var argument = "/select, \"" + fileName + "\"";
                        Process.Start("explorer.exe", argument);
                    }
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            LogMessageOnListBox($"Attempting geom log deletion.", _logEntries, listBoxLog, MaxLogEntries);
            var files = GetIfcFiles(txtSources.Text).ToList();
            int i = 0;
            foreach (var file in files)
            {
                var dir = file.Directory;
                if (dir is null)
                    continue;
                var rep = Path.ChangeExtension(file.Name, ".geom*.log");
                var path = file.Name;
                var logs = dir.GetFiles(rep);
                foreach (var log in logs)
                {
                    LogMessageOnListBox($"Deleted: {log.FullName}", _logEntries, listBoxLog, MaxLogEntries);
                    log.Delete();
                    i++;
                }
            }
            LogMessageOnListBox($"Deleted {i} log files.", _logEntries, listBoxLog, MaxLogEntries);
        }

        private void listBoxLog_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete)
            {
                _logEntries.Clear();
                listBoxLog.Items.Clear();
            }
        }

        private void cmdMoveUp_Click(object sender, EventArgs e)
        {
            if (lstSequence.SelectedItems.Count != 1)
                return;
            var selIndex = lstSequence.SelectedIndices[0];
            if (selIndex <= 0)
                return; // already at the top
            var item = lstSequence.Items[selIndex];
            lstSequence.Items.RemoveAt(selIndex);
            lstSequence.Items.Insert(selIndex - 1, item);
            lstSequence.SelectedItems.Clear();
            lstSequence.SelectedIndices.Add(selIndex - 1);
        }

        private void cmdMoveDown_Click(object sender, EventArgs e)
        {
            if (lstSequence.SelectedItems.Count != 1)
                return;
            var selIndex = lstSequence.SelectedIndices[0];
            if (selIndex > lstSequence.Items.Count - 2)
                return; // already at the bottom
            var item = lstSequence.Items[selIndex];
            lstSequence.Items.RemoveAt(selIndex);
            lstSequence.Items.Insert(selIndex + 1, item);
            lstSequence.SelectedItems.Clear();
            lstSequence.SelectedIndices.Add(selIndex + 1);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            _meshHelper.RequestDumpStack();
        }

        private void cmdEnumerateFiles_Click(object sender, EventArgs e)
        {
            var t = GetFiles();
            LogMessageOnListBox($"{t.Count} files to process.", _logEntries, listBoxLog, MaxLogEntries);
            foreach (var item in t)
            {
                LogMessageOnListBox($"{item.FullName}", _logEntries, listBoxLog, MaxLogEntries);
            }
        }
    }

    /// <summary>
    /// helper extension methods for Control
    /// </summary>
    public static class ControlExtensions
    {
        /// <summary>
        /// determine if thread invoking is required for an action
        /// </summary>
        public static void InvokeIfRequired<T>(this T control, Action<T> action) where T : Control
        {
            if (control.InvokeRequired)
            {
                control.Invoke(action, control);
            }
            else
            {
                action(control);
            }
        }
    }
}
