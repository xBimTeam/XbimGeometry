using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

namespace XbimInstanceLeakTest
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [TestClass]
    public class LeakageTest
    {
        const int numThreads = 10;
        const int numTasks = 5000;
        static int started = 0;
        static int processed = 0;
        static object locker = new object();

        static ConcurrentBag<string> allFiles = new ConcurrentBag<string>();
        static ConcurrentBag<string> results = new ConcurrentBag<string>();        

        // this test tries to mesh files 5000 times to spot cross thread leaks
        //

        [TestMethod]
        public void Exec()
        {
            // this test is based on private files.
            // it will just pass if the files are not available.
            //

            var dir = @"C:\Users\sgmk2\OneDrive\CloudIfcArchive\VP";
            if (!Directory.Exists(dir))
                return;

            var files = Directory.EnumerateFiles(dir);


            Directory.CreateDirectory("Temp");

            foreach (var file in files)
            {
                allFiles.Add(file);
                ProcessIFC(file, $"INITIAL");
            }

            List<Thread> threads = new List<Thread>();

            for (int i=0; i< numThreads; ++i)
            {
                threads.Add(RunInThread());
            }

            foreach (var t in threads)
            {
                t.Join();
            }

        }

        static Thread RunInThread()
        {
            var t = new Thread(() =>
            {
                var rnd = new Random();

                while (processed < numTasks)
                {
                    int task;
                    lock (locker)
                    {
                        task = started++;
                    }

                    string sourceFile = allFiles.ElementAt(task % allFiles.Count());

                    var result = ProcessIFC(sourceFile, "THREADED");

                    lock (locker)
                    {
                        Debug.WriteLine($"[{processed++}/{numTasks}]{result}");
                    }
                }
            });
            t.Start();
            return t;
        }

        private static string ProcessIFC(string sourceFile, string prefix)
        {
            var tempFolder = $@"Temp\{Guid.NewGuid()}";
            Directory.CreateDirectory(tempFolder);
            Debug.WriteLine("Folder: " + tempFolder);

            var ifcFile = $@"{tempFolder}\{Path.GetFileName(sourceFile)}";

            File.Copy(sourceFile, ifcFile);

            Debug.WriteLine($"Processing {ifcFile} [ {sourceFile} ]");


            string ex = "";
            uint hc = 0;
            int sc = 0;

            try
            {
                using (IfcStore model = IfcStore.Open(ifcFile))
                {

                    var m3D = new Xbim3DModelContext(model);
                    try
                    {
                        m3D.CreateContext(adjustWcs: false);
                    }
                    catch (Exception e)
                    {
                        // we know of a permission denied Esent issue with the firestation that needs to be
                        // investigated separately
                        //
                        ex = $"EXCEPTION: " + e.Message;
                    }

                    var shapes = m3D.ShapeGeometries();

                    sc = shapes.Count();

                    hc = 0;
                    foreach (var s in shapes)
                    {
                        hc ^= (uint)s.ShapeData.GetHashCode();
                    }

                }
            }
            catch (Exception e)
            {
                ex = $"EXCEPTION: " + e.Message;
            }

            // Are we using the hashcode to identify problems?
            // Can this be coded in the test explicitly?
            // 
            var result = $"{prefix},{ifcFile},{sc},{hc},{ex}";

            results.Add(result);

            Debug.WriteLine($"Finished processing {ifcFile} [ {sourceFile} ]");
            Directory.Delete(tempFolder, true);

            lock (locker)
            {
                // why are we writing all the results every time rather than appending?
                //
                File.WriteAllLines("Result.csv", results.ToArray());
            }

            return result;

        }
    }
}
