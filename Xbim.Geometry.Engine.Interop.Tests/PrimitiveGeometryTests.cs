using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    public class PrimitiveGeometryTests
    {


        static private ILogger logger;
        static private ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
          
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            logger = null;
        }

        //[TestMethod]
        //public void can_build_composite_curve()
        //{
        //    using (var model = MemoryModel.OpenRead(@".\\TestFiles\Primitives\\composite_curve.ifc"))
        //    {
        //        var compCurve = model.Instances.OfType<IIfcCompositeCurve>().FirstOrDefault();
        //        Assert.IsNotNull(compCurve);
        //        var face = geomEngine.CreateFace(compCurve);
        //        Assert.IsNotNull(face.OuterBound);
        //    }
        //}
        [DataTestMethod]
        [DataRow(XGeometryEngineVersion.V5, DisplayName = "V5 Engine")]
        [DataRow(XGeometryEngineVersion.V6, DisplayName = "V6 Engine")]
        public void can_build_ifc_faceted_brep(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\ifc_faceted_brep.ifc"))
            {
                var shape = model.Instances.OfType<IIfcFacetedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geomEngine = XbimGeometryEngine.CreateGeometryEngine(engineVersion, model, loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, logger);
                geom.Count.Should().Be(1);
                geom.First().Volume.Should().BeApproximately(3232.386, 5e-3);
            }
        }
        [DataTestMethod]
        [DataRow(XGeometryEngineVersion.V5, DisplayName = "V5 Engine")]
        [DataRow(XGeometryEngineVersion.V6, DisplayName = "V6 Engine")]
        public void can_build_closed_shell(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\faulty_closed_shell.ifc"))
            {
                var shape = model.Instances.OfType<IIfcClosedShell>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geomEngine = XbimGeometryEngine.CreateGeometryEngine(engineVersion, model, loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, logger).FirstOrDefault();
                geom.Volume.Should().BeApproximately(-136033.82966702414,1e-5);
            }
        }
        [DataTestMethod]
        [DataRow(XGeometryEngineVersion.V5, DisplayName = "V5 Engine")]
        [DataRow(XGeometryEngineVersion.V6, DisplayName = "V6 Engine")]
        public void can_build_poorly_aligned_planar_faces(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Primitives\poor_face_planar_fidelity.ifc"))
            {
                var shape = model.Instances.OfType<IIfcClosedShell>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geomEngine = XbimGeometryEngine.CreateGeometryEngine(engineVersion, model, loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, logger).FirstOrDefault();
                Assert.IsNotNull(geom, "This should not fail");
            }
            if(engineVersion==XGeometryEngineVersion.V6)Assert.Inconclusive("V6 produces a different result from V5, investigation is required");
        }

       
    }
}
