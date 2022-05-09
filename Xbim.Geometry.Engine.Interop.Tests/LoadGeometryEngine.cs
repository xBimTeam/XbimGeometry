using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Reflection;
using Xbim.Ifc4.GeometryResource;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class LoadGeometryEngine
    {
        [TestMethod]
        public void SimpleLoad()
        {
            var ge = new XbimGeometryEngine();
            Assert.IsNotNull(ge);

        }

        //[TestMethod]
        //public void LoadFromPath()
        //{
        //    AppDomain dom = AppDomain.CreateDomain("geom");
        //    var tmpDllPath = Path.Combine(Environment.CurrentDirectory, "TestGeom");
        //    var dll32 = Path.Combine(Environment.CurrentDirectory, "Xbim.Geometry.Engine32.dll", Path.Combine(tmpDllPath, "Xbim.Geometry.Engine32.dll"));
        //    var dll64 = Path.Combine(Environment.CurrentDirectory, "Xbim.Geometry.Engine64.dll", Path.Combine(tmpDllPath, "Xbim.Geometry.Engine64.dll"));
        //    Directory.CreateDirectory(tmpDllPath);
        //    if (!File.Exists(dll32))
        //        File.Copy(Path.Combine(Environment.CurrentDirectory, dll32), Path.Combine(tmpDllPath, dll32));
        //    if (!File.Exists(dll64))
        //        File.Copy(Path.Combine(Environment.CurrentDirectory, dll64), Path.Combine(tmpDllPath, dll64));
        //    Environment.SetEnvironmentVariable("GeometryEngineLocation", tmpDllPath);
        //    AssemblyName assemblyName = new AssemblyName();
        //    assemblyName.CodeBase = Path.Combine(Environment.CurrentDirectory, "Xbim.Geometry.Engine.Interop.dll");
        //    dom.Load(assemblyName);
        //    var ge = new XbimGeometryEngine();
        //    var anyObject = ge.CreateSolidSet();
        //    Assert.IsTrue(anyObject.GetType().Module.FullyQualifiedName.StartsWith(tmpDllPath));
        //    AppDomain.Unload(dom);
        //}


        [TestMethod]
        public void TestLogging()
        {

           
            var loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            var logger = loggerFactory.CreateLogger<LoadGeometryEngine>();
            var ge = new XbimGeometryEngine();
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("new"))
                {
                    var pline = m.Instances.New<IfcPolyline>();
                    ge.CreateCurve(pline, logger);
                }

            }
        }

       
    }
}
