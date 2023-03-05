using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xunit;
using System.Linq;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.Geometry.Engine.Interop;

namespace Xbim.Geometry.Engine.Tests
{

    // [DeploymentItem("TestFiles")]
    public class IfcAdvancedBrepTests
    {


        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory factory;

        public IfcAdvancedBrepTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _loggerFactory = loggerFactory;
            this.factory = factory;
        }
        [Fact]
        public void IfcAdvancedBrepTrimmedCurveTest()
        {
            using (var er = new EntityRepository<IIfcAdvancedBrep>(nameof(IfcAdvancedBrepTrimmedCurveTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngineV5(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolid(er.Entity);
                solid.Faces.Count.Should().Be(14, "This solid should have 14 faces");
            }

        }

        [Fact]
        public void Incorrectly_defined_edge_curve()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\incorrectly_defined_edge_curve.ifc"))
            {
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                brep.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet(brep);
                solids.Count.Should().Be(3); //this should really be one but the model is incorrect
                solids.First().Faces.Count.Should().Be(60);
            }

        }
        /// <summary>
        /// This test still produces incorrect solids and surfaces, the model appears to be faulty
        /// </summary>
        [Fact]
        public void Incorrectly_defined_edge_curve_with_identical_points()
        {

            using (var model = MemoryModel.OpenRead(@"TestFiles\incorrectly_defined_edge_curve_with_identical_points.ifc"))
            {
                //this model needs workarounds to be applied
                model.AddRevitWorkArounds();
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                brep.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet(brep);
                solids.Count.Should().Be(2);
                solids.First().Faces.Count.Should().Be(8);
            }

        }

        [Theory]
        [InlineData("SurfaceCurveSweptAreaSolid_1", 2944208.3398366235/*, DisplayName = "Handles Self Intersection unorientable shape"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_2", 0.00025657473102144062/*, DisplayName = "Handles Planar reference surface, parallel to sweep"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_3", 0.26111117805532907, false/*, DisplayName = "Reference Model from IFC documentation"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_4", 19.276830224679465/*, DisplayName = "Handles Trimmed directrix is periodic"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_5", 12.603349469526613, false, true/*, DisplayName = "Handles Polylines Incorrectly Trimmed as 0 to 1"*/)]
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
                var geomEngine = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var sweptSolid = geomEngine.CreateSolid(surfaceSweep);
                sweptSolid.Volume.Should().BeApproximately(requiredVolume, 1e-3);
                //var shapeGeom = geomEngine.CreateShapeGeometry(model.ModelFactors.OneMilliMeter,sweptSolid,
                //    model.ModelFactors.Precision, logger);

            }
        }

        [Theory]
        [InlineData("SurfaceCurveSweptAreaSolid_6", 12.603349469526613/*, DisplayName = "Directrix trim incorrectly set to 0, 360 by Revit"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_7", 12.603349469526613, false/*, DisplayName = "Directrix trim from Flex Ifc Exporter trim  set to 270, 360 by Revit"*/)]
        public void SurfaceCurveSweptAreaSolid_Tests_ToFix(string fileName, double requiredVolume, bool addLinearExtrusionWorkAround = true, bool addPolyTrimWorkAround = false)
        {
            using (var model = MemoryModel.OpenRead($@"TestFiles\{fileName}.ifc"))
            {
                if (addLinearExtrusionWorkAround)
                    ((XbimModelFactors)model.ModelFactors).AddWorkAround("#SurfaceOfLinearExtrusion");
                if (addPolyTrimWorkAround)
                    model.AddWorkAroundTrimForPolylinesIncorrectlySetToOneForEntireCurve();
                var surfaceSweep = model.Instances.OfType<IIfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                surfaceSweep.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var sweptSolid = geomEngine.CreateSolid(surfaceSweep);
                sweptSolid.Volume.Should().BeApproximately(requiredVolume, 1e-3);
                //var shapeGeom = geomEngine.CreateShapeGeometry(model.ModelFactors.OneMilliMeter,sweptSolid,
                //    model.ModelFactors.Precision, logger);

            }
        }

        [Fact]
        public void Advanced_brep_with_sewing_issues()
        {

            using (var model = MemoryModel.OpenRead(@"TestFiles\advanced_brep_with_sewing_issues.ifc"))
            {
                //this model needs workarounds to be applied
                // model.AddWorkAroundSurfaceofLinearExtrusionForRevit();
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                brep.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet(brep);
                var shapeGeom = geomEngine.CreateShapeGeometry(solids,
                    model.ModelFactors.Precision, model.ModelFactors.DeflectionTolerance,
                    model.ModelFactors.DeflectionAngle, XbimGeometryType.PolyhedronBinary);
                solids.Count.Should().Be(2);
                solids.First().Faces.Count.Should().Be(10);
            }

        }


        //[Theory]
        //[InlineData("ShapeGeometry_5")]
        //[InlineData("ShapeGeometry_6")]
        //[InlineData("ShapeGeometry_18")]
        //[InlineData("ShapeGeometry_20")]
        //public void Advanced_brep_shapes(string fileName)
        //{
        //    using (var model = MemoryModel.OpenRead($@"C:\Users\Steve\Documents\testModel\{fileName}.ifc"))
        //    {
        //        foreach (var advBrep in model.Instances.OfType<IIfcAdvancedBrep>())
        //        {
        //            var solids = geomEngine.CreateSolidSet(advBrep, logger);
        //            solids.Count > 0);
        //        }
        //    }
        //}


        //This is a fauly Brep conversion case that needs t be firther examinedal
        [Theory]
        [InlineData("advanced_brep_1", 1/*, DisplayName = "Self Intersection unorientable shape"*/)]
        [InlineData("advanced_brep_2"/*, DisplayName = "Curved edges with varying orientation"*/)]
        [InlineData("advanced_brep_3"/*, DisplayName = "Badly formed wire orders and missing faces and holes"*/)]
        [InlineData("advanced_brep_4", 2/*, DisplayName = "Two solids from one advanced brep, errors in holes"*/)]
        [InlineData("advanced_brep_5", 1/*, DisplayName = "Example of arc and circle having centre displaced twice RevitIncorrectArcCentreSweptCurve"*/)]
        [InlineData("advanced_brep_6", 1/*, DisplayName = "The trimming points either result in a zero length curve or do not intersect the curve"*/)]
        [InlineData("advanced_brep_7", 2/*, DisplayName = "Long running construction"*/)]
        [InlineData("advanced_brep_8", 2/*, DisplayName = "BSpline with displacement applied twice, example of RevitIncorrectBsplineSweptCurve"*/)]
        public void Advanced_brep_tests(string brepFileName, int solidCount = 1, bool fails = false)
        {

            using (var model = MemoryModel.OpenRead($@"TestFiles\{brepFileName}.ifc"))
            {
                model.AddRevitWorkArounds();
                //this model needs workarounds to be applied
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                brep.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet(brep);
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
