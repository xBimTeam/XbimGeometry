using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    [TestClass]
    // [DeploymentItem("TestFiles")]
    public class IfcAdvancedBrepTests
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
        public void IfcAdvancedBrepTrimmedCurveTest()
        {
            using (var er = new EntityRepository<IIfcAdvancedBrep>(nameof(IfcAdvancedBrepTrimmedCurveTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcAdvancedBrep found");
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                 Assert.IsTrue(solid.Faces.Count == 14, "This solid should have 14 faces");
            }

        }
        
        [TestMethod]
        public void Incorrectly_defined_edge_curve()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\incorrectly_defined_edge_curve.ifc"))
            {
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep, "No IIfcAdvancedBrep found");
                var solids = geomEngine.CreateSolidSet(brep, logger);
                Assert.IsTrue(solids.Count == 1, "This set should have 2 solids");

               Assert.IsTrue(solids.First().Faces.Count == 62, "This solid should have 62 faces");
            }

        }

        [TestMethod]
        public void Incorrectly_defined_edge_curve_with_identical_points()
        {
            
            using (var model = MemoryModel.OpenRead(@"TestFiles\incorrectly_defined_edge_curve_with_identical_points.ifc"))
            {
                //this model needs workarounds to be applied
                model.AddWorkAroundSurfaceofLinearExtrusionForRevit();
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep, "No IIfcAdvancedBrep found");
                var solids = geomEngine.CreateSolidSet(brep, logger);
                Assert.IsTrue(solids.Count == 2, "This set should have 2 solids");
                Assert.IsTrue(solids.First().Faces.Count == 8, "Solid 0 should have 8 faces");
            }

        }

        [TestMethod]
        public void Swept_curve_with_bad_trims()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\swept_curve_with_bad_trims.ifc"))
            {
                var sweptSolid = model.Instances.OfType<IIfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                bool wa = model.ModelFactors.ApplyWorkAround("#SurfaceOfLinearExtrusion");
                Assert.IsNotNull(sweptSolid);
                var solid = geomEngine.CreateSolid(sweptSolid);
                //this solid is invalid
                Assert.IsFalse(solid.IsValid);
              
            }
        }

        [TestMethod]
        public void Advanced_brep_with_sewing_issues()
        {

            using (var model = MemoryModel.OpenRead(@"TestFiles\advanced_brep_with_sewing_issues.ifc"))
            {
                //this model needs workarounds to be applied
                model.AddWorkAroundSurfaceofLinearExtrusionForRevit();
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep, "No IIfcAdvancedBrep found");
                var solids = geomEngine.CreateSolidSet(brep, logger);
                var shapeGeom = geomEngine.CreateShapeGeometry(solids,
                    model.ModelFactors.Precision, model.ModelFactors.DeflectionTolerance, 
                    model.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary, logger);
                Assert.IsTrue(solids.Count == 2, "This set should have 2 solids");
                Assert.IsTrue(solids.First().Faces.Count == 37, "Solid 0 should have 37 faces");
            }

        }
    }
}
