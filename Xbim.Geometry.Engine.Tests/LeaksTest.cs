using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

namespace XbimInstanceLeakTest
{
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

        [TestMethod]
        public void Exec()
        {
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
                        Console.WriteLine($"[{processed++}/{numTasks}]{result}");
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

            var ifcFile = $@"{tempFolder}\{Path.GetFileName(sourceFile)}";

            File.Copy(sourceFile, ifcFile);

            Console.WriteLine($"Processing {ifcFile} [ {sourceFile} ]");


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

            var result = $"{prefix},{ifcFile},{sc},{hc},{ex}";

            results.Add(result);

            Console.WriteLine($"Finished processing {ifcFile} [ {sourceFile} ]");
            Directory.Delete(tempFolder, true);

            lock (locker)
            {
                File.WriteAllLines("Result.csv", results.ToArray());
            }

            return result;

        }
    }
}
