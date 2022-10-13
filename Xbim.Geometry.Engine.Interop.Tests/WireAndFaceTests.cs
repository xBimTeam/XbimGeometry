using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
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
    // [DeploymentItem("TestFiles")]
    public class WireAndFaceTests
    {
       
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            logger = new NullLogger<IfcAdvancedBrepTests>();
           
        }
        [ClassCleanup]
        static public void Cleanup()
        {
           
            
            logger = null;
        }

        [TestMethod]
        public void Empty_Polyline()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Polyline.ifc"))
            {
                var poly = model.Instances.OfType<IIfcPolyline>().FirstOrDefault();
                var geomEngine = new XbimGeometryEngine(model, logger);
                var wire = geomEngine.CreateWire(poly, logger);
            }
        }
        [TestMethod]
        public void Composite_curve_issue_261()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Composite_curve_issue_261.ifc"))
            {
                var composite_curve = model.Instances.OfType<IIfcCompositeCurve>().FirstOrDefault();
                var geomEngine = new XbimGeometryEngine(model, logger);
                var wire = geomEngine.CreateWire(composite_curve, logger);
            }
        }

        [TestMethod]
        public void TestIfFaceIsPlanar()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\non_planar_wire.ifc"))
            {
                var polyloop = model.Instances.OfType<IIfcPolyLoop>().FirstOrDefault();
                Assert.IsNotNull(polyloop);
                var geomEngine = new XbimGeometryEngine(model, logger);
                var face = geomEngine.CreateFace(polyloop, logger);
                Assert.IsNotNull(face.OuterBound);
            }

        }

    }
}
