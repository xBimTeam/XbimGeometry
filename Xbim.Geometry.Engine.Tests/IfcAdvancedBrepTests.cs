using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xunit;
using System.Linq;
using Xbim.Common;
using Xbim.Common.Geometry;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Exceptions;
using Xbim.Geometry.Abstractions;

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
        [InlineData("SurfaceCurveSweptAreaSolid_1", 5888416.692/*, DisplayName = "Handles Swepted elipse, sweep parameters override directrix trims"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_2", 0.0, true, false, true/*, DisplayName = "Handles  reference surface incorrectly parallel to sweep"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_3", 0.26111117805532907, false/*, DisplayName = "Reference Model from IFC documentation"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_4", 19.276830224679465/*, DisplayName = "Handles Trimmed directrix is periodic"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_5", 12.603349469526613, false, true/*, DisplayName = "Handles Polylines Incorrectly Trimmed as 0 to 1"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_6", 333574/*, DisplayName = "Directrix trim incorrectly set to 0, 360 by Revit, creates a sphere"*/)]
        [InlineData("SurfaceCurveSweptAreaSolid_7", 760884, false/*, DisplayName = "Directrix trim from Flex Ifc Exporter trim  set to 270, 360 by Revit. Creates a 90 deg elbow"*/)]

        public void SurfaceCurveSweptAreaSolid_Tests(string fileName, double requiredVolume, bool addLinearExtrusionWorkAround = true, bool addPolyTrimWorkAround = false, bool throwsException = false)
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
                if (throwsException)
                {
                    var ex = Assert.Throws<XbimGeometryFactoryException>(() => geomEngine.CreateSolid(surfaceSweep));
                    ex.Message.Should().Be("Failure building SurfaceCurveSweptAreaSolid");
                }
                else
                {
                    var sweptSolid = geomEngine.CreateSolid(surfaceSweep);
                    sweptSolid.Volume.Should().BeApproximately(requiredVolume, 1);
                }

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
        [InlineData("advanced_brep_1", 1, 1,2445135, 2445135/*, DisplayName = "Self Intersection unorientable shape"*/)]
        [InlineData("advanced_brep_2", 1, 1,828514, 828514 /*, DisplayName = "Curved edges with varying orientation"*/)]
        [InlineData("advanced_brep_3", 1, 1,2466953, 2077748/*, DisplayName = "Badly formed wire orders and missing faces and holes, accurate in V6 but still bad definition"*/)]
        [InlineData("advanced_brep_4", 2, 1,864225, 830475/*, DisplayName = "Two solids from one advanced brep, errors in holes"*/)]
        [InlineData("advanced_brep_5", 1, 1, 114, 114/*, DisplayName = "Example of arc and circle having centre displaced twice RevitIncorrectArcCentreSweptCurve"*/)]
        [InlineData("advanced_brep_6", 1, 1, 3246676, 8192511/*, DisplayName = "The top face of the sink does not have a hole defined in it, fault model. V6 is truer"*/)]
        [InlineData("advanced_brep_7", 2, 1, 1821558, 1819075/*, DisplayName = "Pipe unit built as 2 pieces in V5, V6 correctly build to one piece"*/)]
        [InlineData("advanced_brep_8", 2, 1, 53286, 52249/*, DisplayName = "BSpline with displacement applied twice, example of RevitIncorrectBsplineSweptCurve, V6 corrects dual solids"*/)]
        public void Advanced_brep_tests(string brepFileName, int v5SolidCount, int v6SolidCount, double volumeV5, double volumeV6)
        {

            using (var model = MemoryModel.OpenRead($@"TestFiles\{brepFileName}.ifc"))
            {
                model.AddRevitWorkArounds();
                //this model needs workarounds to be applied
                var brep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                brep.Should().NotBeNull();
                var geomEngineV5 = factory.CreateGeometryEngineV5(model, _loggerFactory);
                var solidsV5 = geomEngineV5.Create(brep) as IXbimGeometryObjectSet;
                solidsV5.IsValid.Should().BeTrue();
                solidsV5.Should().HaveCount(v5SolidCount);
                volumeV5.Should().BeApproximately(solidsV5.Cast<IXbimSolid>().Sum(s => s.Volume), 1);
                //repeat with V6
                var geomEngineV6 = factory.CreateGeometryEngineV6(model, _loggerFactory);
                if(v6SolidCount > 1)
                {
                    var solidsV6 = geomEngineV6.Build(brep) as IXCompound;
                    solidsV6.Should().NotBeNull("This brep should be a multiple solid");
                    solidsV6.IsSolidsOnly.Should().BeTrue();
                    solidsV6.Solids.Should().HaveCount(v6SolidCount);
                    volumeV6.Should().BeApproximately(solidsV6.Solids.Cast<IXSolid>().Sum(s => s.Volume), 1);
                }
                else
                {
                    var solidsV6 = geomEngineV6.Build(brep) as IXSolid;
                    solidsV6.Should().NotBeNull("This brep should be a single solid");
                    volumeV6.Should().BeApproximately(solidsV6.Volume, 1);
                }
            }
        }
    }
}
