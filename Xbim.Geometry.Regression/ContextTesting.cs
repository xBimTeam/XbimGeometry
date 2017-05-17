using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Parser;
using Xbim.ModelGeometry.Scene;

namespace XbimRegression
{
    /// <summary>
    /// temporarily created to test multiple files
    /// </summary>
    internal class ContextTesting
    {
        public static void Run()
        {
            var d = new DirectoryInfo("C:\\Users\\Claudio\\Dropbox");
            // var d = new DirectoryInfo("C:\\Users\\Claudio\\OneDrive\\IfcArchive");
            // var d = new DirectoryInfo("C:\\Users\\Claudio\\Dropbox\\IfcModels\\Slow Xbim3DModelContext");
            
            var files = new List<FileInfo>();
            files.AddRange(d.GetFiles("*.ifc", SearchOption.AllDirectories));
            files.AddRange(d.GetFiles("*.xbim", SearchOption.AllDirectories));

            var logname = Path.Combine(d.FullName, "log.log");

            using (var log = File.CreateText(logname))
            {
                foreach (var fileInfo in files)
                {
                    try
                    {
                        using (var model = IfcStore.Open(fileInfo.FullName, null,-1))
                        {
                            var watch = new Stopwatch();
                            watch.Start();
                            var c = new Xbim3DModelContext(model);
                            watch.Stop();

                            var elabs = c.Contexts.Select(x => x.EntityLabel).ToList();
                            var ctUnsorted = string.Join(",", elabs);
                            elabs.Sort();
                            var ctSorted = string.Join(",", elabs);

                            var all = model.Instances.OfType<IIfcGeometricRepresentationContext>()
                                .Select(x => x.EntityLabel).ToList();
                            all.Sort();
                            var allSorted = string.Join(",", all);
                            log.WriteLine(string.Format("{0}\t{1}\t{2}\t{3}", fileInfo.FullName, ctUnsorted, ctSorted,
                                allSorted));
                            log.Flush();
                        }
                    }
                    catch (Exception e)
                    {
                        log.WriteLine(string.Format("{0}\t{1}", fileInfo.FullName, e.Message));
                        log.Flush();
                    }
                }
            }
        }
    }
}