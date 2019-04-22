using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class MemoryAndThreadingTests
    {
        static private IXbimGeometryEngine geomEngine;


        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            geomEngine = new XbimGeometryEngine();
        }
        [ClassCleanup]
        static public void Cleanup()
        {

            geomEngine = null;

        }

       

        [TestMethod]
        public void simple_thread_time_out_operation_test()
        {
            var runCount = 20;
            var res = new ConcurrentDictionary<int, long>();
            ParallelOptions po = new ParallelOptions();
            //po.MaxDegreeOfParallelism = 4;
            var totalTime = new Stopwatch();
            totalTime.Start();
            //get a benchmark
            for (int i = 0; i < runCount; i++)
            {

                var sw = new Stopwatch();
                sw.Start();
                try
                {
                    Sleep();
                    //var t = new Task(()=>Sleep(), tokenSource.Token);
                    //t.Wait(tokenSource.Token);
                    // t.RunSynchronously();
                    sw.Stop();
                    res.TryAdd(i, sw.ElapsedMilliseconds);
                }               
                catch (TimeoutException)
                {
                    sw.Stop();
                    res.TryAdd(-i, sw.ElapsedMilliseconds);
                }

            }

            int avTime = (int)res.Average(r => r.Value);
            res.Clear();
            var firstRunTime = totalTime.ElapsedMilliseconds;
            totalTime.Restart();
            Parallel.For(0, runCount, ( i) =>
             {
                 

                 var sw = new Stopwatch();
                 sw.Start();
                 try
                 {
                     var time = CallWithTimeout(Sleep, SleepTime * 3);

                     //var t = new Task(()=>Sleep(), tokenSource.Token);
                     //t.Wait(tokenSource.Token);
                     // t.RunSynchronously();
                     sw.Stop();
                     res.TryAdd(i, sw.ElapsedMilliseconds);
                 }                 
                 catch (TimeoutException)
                 {
                     sw.Stop();
                     res.TryAdd(-i, sw.ElapsedMilliseconds);
                 }
  
             });
            totalTime.Stop();
            var secondRunTime = totalTime.ElapsedMilliseconds;
            Assert.IsTrue(res.Where(kv => kv.Key < 0).Any(),"Some executions should have timed out");
            Console.WriteLine($"Single thread simple execution time {firstRunTime}, av unit {avTime}ms");
            Console.WriteLine($"Multi thread with time out run time {secondRunTime}, av unit {(int)res.Average(r => r.Value)}ms, number of time outs {res.Count(kv=> kv.Key<0)} out of {runCount}");
        }

        static T CallWithTimeout<T>(Func<T> action, int timeoutMilliseconds)
        {
            //Thread threadToKill = null;
            Func<T> wrappedAction = () =>
            {
               return action();          
            };

            IAsyncResult result = wrappedAction.BeginInvoke(null, null);
            if (result.AsyncWaitHandle.WaitOne(timeoutMilliseconds))
            {
                return wrappedAction.EndInvoke(result);
            }
            else
            {
               // threadToKill.Abort();
                throw new TimeoutException();
            }
            
        }
        const int SleepTime = 50;
        static int Sleep()
        {
            Thread.Sleep(SleepTime);
            return SleepTime;
        }

        [TestMethod]
        public void simple_vertex_is_constructed_and_disposed()
        {
            //IXbimVertex vertex;
            {
                var aPoint = new Common.Geometry.XbimPoint3D(1, 2, 3);
                //using (var pt = geomEngine.CreateVertexPoint(aPoint, 0.005))
                //{
                //    vertex = pt;
                //    Assert.AreEqual(pt.VertexGeometry.X, aPoint.X);
                //    Assert.AreEqual(pt.VertexGeometry.Y, aPoint.Y);
                //    Assert.AreEqual(pt.VertexGeometry.Z, aPoint.Z);
                //}
                //Assert.IsFalse(vertex.IsValid);
                var vertices = new List<IXbimVertex>(10000);
                for (int i = 0; i < 1000000; i++)
                {
                    vertices.Add(geomEngine.CreateVertexPoint(aPoint, 0.005));
                }
                vertices = null;
                GC.Collect();
            }
        }

    }
}
