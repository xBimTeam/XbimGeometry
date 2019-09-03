using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    [DeploymentItem("TestFiles")]
    public class WireAndFaceTests
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<IfcAdvancedBrepTests>();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }
        [TestMethod]
        public void TestIfFaceIsPlanar()
        {
            using (var model = MemoryModel.OpenRead(@"non_planar_wire.ifc"))
            {
                var polyloop = model.Instances.OfType<IIfcPolyLoop>().FirstOrDefault();
                Assert.IsNotNull(polyloop);
                var face = geomEngine.CreateFace(polyloop);
                Assert.IsNotNull(face.OuterBound);
            }

        }
    }
}
