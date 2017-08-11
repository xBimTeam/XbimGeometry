using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
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
        [TestMethod]
        public void TestLogging()
        {


            Xbim.Common.ApplicationLogging.LoggerFactory.AddConsole(LogLevel.Trace);
            var logger = Xbim.Common.ApplicationLogging.LoggerFactory.CreateLogger<LoadGeometryEngine>();
            var ge = new XbimGeometryEngine();
            using (var m = new MemoryModel(new Ifc4.EntityFactory()))
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
