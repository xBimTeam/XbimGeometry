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
using FluentAssertions;
using Xbim.Common;

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
                brep.Should().NotBeNull();

                var solids = geomEngine.CreateSolidSet(brep, logger);
                solids.Count.Should().Be(3); //this should really be one but the model is incorrect
                solids.First().Faces.Count.Should().Be(60);
            }

        }
        /// <summary>
        /// This test still produces incorrect solids and surfaces, the model appears to be faulty
        /// </summary>
        [TestMethod]
        public void Incorrectly_defined_edge_curve_with_identical_points()
        {

            using (var model = MemoryModel.OpenRead(@"TestFiles\incorrectly_defined_edge_curve_with_identical_points.ifc"))
            {
                //this model needs workarounds to be applied
                model.AddRevitWorkArounds();
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep, "No IIfcAdvancedBrep found");
                var solids = geomEngine.CreateSolidSet(brep, logger);
                solids.Count.Should().Be(2);
                solids.First().Faces.Count.Should().Be(8);
            }

        }

        [DataTestMethod]
        [DataRow("SurfaceCurveSweptAreaSolid_1", 29.444264292193111, DisplayName = "Handles Self Intersection unorientable shape")]
        [DataRow("SurfaceCurveSweptAreaSolid_2", 0.00025657473102144062, DisplayName = "Handles Planar reference surface, parallel to sweep")]
        [DataRow("SurfaceCurveSweptAreaSolid_3", 0.26111117805532907, false, DisplayName = "Reference Model from IFC documentation")]
        [DataRow("SurfaceCurveSweptAreaSolid_4", 19.276830224679465, DisplayName = "Handles Trimmed directrix is periodic")]
        [DataRow("SurfaceCurveSweptAreaSolid_5", 12.603349469526613, false, true, DisplayName = "Handles Polylines Incorrectly Trimmed as 0 to 1")]
        [DataRow("SurfaceCurveSweptAreaSolid_6", 12.603349469526613, DisplayName = "Directrix trim incorrectly set to 0, 360 by Revit")]
       // [DataRow("SurfaceCurveSweptAreaSolid_7", 12.603349469526613,false, DisplayName = "Directrix trim from Flex Ifc Exporter trim  set to 270, 360 by Revit")]
        public void SurfaceCurveSweptAreaSolid_Tests(string fileName, double requiredVolume, bool addLinearExtrusionWorkAround = true, bool addPolyTrimWorkAround = false)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                if (addLinearExtrusionWorkAround)
                    ((XbimModelFactors)model.ModelFactors).AddWorkAround("#SurfaceOfLinearExtrusion");
                if(addPolyTrimWorkAround)
                    model.AddWorkAroundTrimForPolylinesIncorrectlySetToOneForEntireCurve();
                var surfaceSweep = model.Instances.OfType<IIfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                surfaceSweep.Should().NotBeNull();
                var sweptSolid = geomEngine.CreateSolid(surfaceSweep);
                //sweptSolid.Volume.Should().BeApproximately(requiredVolume, 1e-7);
                //var shapeGeom = geomEngine.CreateShapeGeometry(model.ModelFactors.OneMilliMeter,sweptSolid,
                //    model.ModelFactors.Precision, logger);
               
            }
        }

        [TestMethod]
        public void Advanced_brep_with_sewing_issues()
        {

            using (var model = MemoryModel.OpenRead(@"TestFiles\advanced_brep_with_sewing_issues.ifc"))
            {
                //this model needs workarounds to be applied
               // model.AddWorkAroundSurfaceofLinearExtrusionForRevit();
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep, "No IIfcAdvancedBrep found");
                var solids = geomEngine.CreateSolidSet(brep, logger);
                var shapeGeom = geomEngine.CreateShapeGeometry(solids,
                    model.ModelFactors.Precision, model.ModelFactors.DeflectionTolerance,
                    model.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary, logger);
                solids.Count.Should().Be(2);
                solids.First().Faces.Count.Should().Be(10);
            }

        }


        //[DataTestMethod]
        //[DataRow("ShapeGeometry_5")]
        //[DataRow("ShapeGeometry_6")]
        //[DataRow("ShapeGeometry_18")]
        //[DataRow("ShapeGeometry_20")]
        //public void Advanced_brep_shapes(string fileName)
        //{
        //    using (var model = MemoryModel.OpenRead($@"C:\Users\Steve\Documents\testModel\{fileName}.ifc"))
        //    {
        //        foreach (var advBrep in model.Instances.OfType<IIfcAdvancedBrep>())
        //        {
        //            var solids = geomEngine.CreateSolidSet(advBrep, logger);
        //            Assert.IsTrue(solids.Count > 0);
        //        }
        //    }
        //}


        //This is a fauly Brep conversion case that needs t be firther examinedal
        [DataTestMethod]
        [DataRow("advanced_brep_1", false, 1, DisplayName = "Self Intersection unorientable shape")]
        [DataRow("advanced_brep_2", DisplayName = "Curved edges with varying orientation")]
        [DataRow("advanced_brep_3", false, DisplayName = "Badly formed wire orders and missing faces and holes")]
        [DataRow("advanced_brep_4", true, 2, DisplayName = "Two solids from one advanced brep, errors in holes")]
        [DataRow("advanced_brep_5", true, 1, DisplayName = "Example of arc and circle having centre displaced twice RevitIncorrectArcCentreSweptCurve")]
        [DataRow("advanced_brep_6", true, 1, DisplayName = "The trimming points either result in a zero length curve or do not intersect the curve")]
        [DataRow("advanced_brep_7", true, 2, DisplayName = "Long running construction")]
        [DataRow("advanced_brep_8", true, 2, DisplayName = "BSpline with displacement applied twice, example of RevitIncorrectBsplineSweptCurve")]
        public void Advanced_brep_tests(string brepFileName, bool isValidSolid = true, int solidCount = 1, bool fails = false)
        {

            using (var model = MemoryModel.OpenRead($@"TestFiles\{brepFileName}.ifc"))
            {
                model.AddRevitWorkArounds();
                //this model needs workarounds to be applied
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                brep.Should().NotBeNull();
                var solids = geomEngine.CreateSolidSet(brep, logger);
                solids.IsValid.Should().BeTrue();
                solids.Should().HaveCount(solidCount);
                if (!fails) //if we fail the volume will be 0 or less
                {
                    foreach (var solid in solids)
                    {
                        solid.Volume.Should().BeGreaterThan(0);
                    }
                }

            }

        }
    }
}
