using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using System;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Exceptions;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Interop.Tests
{


    public class Ifc4GeometryTests
    {



        static private ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory factory;
        static ILogger _logger;

        public Ifc4GeometryTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _loggerFactory = loggerFactory;
            this.factory = factory;
            _logger = _loggerFactory.CreateLogger<Ifc4GeometryTests>();
        }
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Can_build_ifcadvancedbrep_with_faulty_surface_orientation(XGeometryEngineVersion engineVersion)
        {

            using (var model = MemoryModel.OpenRead(@"testfiles/ifcadvancedbrep_with_faulty_surface_orientation.ifc"))
            {
                //MemoryModel.SetWorkArounds(model.Header, model.ModelFactors as XbimModelFactors);
                var advancedBrep = model.Instances.OfType<IIfcAdvancedBrep>().FirstOrDefault();
                if (engineVersion == XGeometryEngineVersion.V5) model.AddRevitWorkArounds();
                advancedBrep.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var solid = geomEngine.CreateSolid(advancedBrep, _logger);

                solid.Volume.Should().BeApproximately(102264692.6969, 1e-4);
                solid.Faces.Count.Should().Be(14);
            }
        }


        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Can_build_polygonal_face_tessellation(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\polygonal-face-tessellation.ifc"))
            {
                var pfs = model.Instances.OfType<IIfcPolygonalFaceSet>().FirstOrDefault();
                pfs.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var faceModel = geomEngine.CreateSurfaceModel(pfs, _logger).OfType<IXbimShell>().FirstOrDefault();
                faceModel.Should().NotBeNull();
                faceModel.Faces.Count.Should().Be(11);

            }
        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Can_build_polygonal_faceset_as_solid(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\polygonal-face-tessellation.ifc"))
            {
                var pfs = model.Instances.OfType<IIfcPolygonalFaceSet>().FirstOrDefault();
                pfs.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var solidModel = geomEngine.Create(pfs, _logger) as IXbimSolidSet;

                solidModel.Should().NotBeNull();
                solidModel.First().Faces.Count.Should().Be(11);
                solidModel.First().Volume.Should().BeApproximately(6500000000000.001, 1e-5);

            }
        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void Composite_curve_with_disconnection(XGeometryEngineVersion engineVersion)
        {
            using (var er = new EntityRepository<IIfcCompositeCurve>(nameof(Composite_curve_with_disconnection)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, er.Model, _loggerFactory);
                var face = geomEngine.CreateFace(er.Entity, _logger);
                face.Area.Should().BeApproximately(22084775, 1);

            }
        }

        [Theory]
        // [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void CentreLineProfileTest(XGeometryEngineVersion engineVersion)
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(CentreLineProfileTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, er.Model, _loggerFactory);
                var extrudedSolid = geomEngine.CreateSolid(er.Entity, _logger);
                HelperFunctions.IsValidSolid(extrudedSolid);
            }
        }

        [Fact]
        public void TrimmedCurveWithLargeRadianValueTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(TrimmedCurveWithLargeRadianValueTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var extrudedSolid = geomEngine.CreateSolid(er.Entity, _logger);
                HelperFunctions.IsValidSolid(extrudedSolid);
                extrudedSolid.Volume.Should().BeApproximately(14999524619.315742, 1e-5);
            }
        }


        [Fact]
        public void ExtrudedSolidWithNullPositionTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(ExtrudedSolidWithNullPositionTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var extrudedSolid = geomEngine.CreateSolid(er.Entity, _logger);
                HelperFunctions.IsValidSolid(extrudedSolid);

            }
        }
        [Fact]
        public void FacetedBrepIsValidSolidTest()
        {
            using (var er = new EntityRepository<IIfcFacetedBrep>(nameof(FacetedBrepIsValidSolidTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolidSet(er.Entity, _logger).FirstOrDefault();
                HelperFunctions.IsValidSolid(solid);

            }

        }
        [Fact]
        public void closed_shell_is_valid_test()
        {
            using (var er = new EntityRepository<IIfcFacetedBrep>(nameof(closed_shell_is_valid_test)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet(er.Entity, _logger);
                solids.Count.Should().Be(4, "Should return 4 solids");

            }

        }
        [Fact]
        public void FacetedBrepWithFacesOutsideNorlamTolerancesTest()
        {
            using (var er = new EntityRepository<IIfcFacetedBrep>(nameof(FacetedBrepWithFacesOutsideNorlamTolerancesTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolidSet(er.Entity, _logger).FirstOrDefault();
                HelperFunctions.IsValidSolid(solid);

            }

        }
        [Fact]
        public void CsgSolidIsValidSolidTest()
        {
            using (var er = new EntityRepository<IIfcCsgSolid>(nameof(CsgSolidIsValidSolidTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolidSet(er.Entity, _logger).FirstOrDefault();
                HelperFunctions.IsValidSolid(solid);

            }
        }
        [Fact]
        public void IndexedPolyCurveTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(IndexedPolyCurveTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolid(er.Entity, _logger);
                HelperFunctions.IsValidSolid(solid);

            }
        }

        [Fact]
        public void TriangulatedFaceSetBasicTest()
        {
            using (var er = new EntityRepository<IIfcTriangulatedFaceSet>(nameof(TriangulatedFaceSetBasicTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var beam = geomEngine.CreateSurfaceModel(er.Entity);
                Math.Abs(beam.BoundingBox.Volume - 20000000).Should().BeLessThan(1);
            }
        }

        [Fact]
        public void CsgSolidBoundingBoxTest()
        {
            using (var er = new EntityRepository<IfcCsgSolid>(nameof(CsgSolidBoundingBoxTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolidSet(er.Entity, _logger).FirstOrDefault();
                Math.Abs(solid.Volume - solid.BoundingBox.Volume).Should().BeLessThan(1e-5);

            }
        }

        [Fact]
        public void ExtrudedAreaSolidBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\extruded-solid.ifc"))
            {
                var eas = model.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                eas.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var solid = geomEngine.CreateSolid(eas, _logger);
                Math.Abs(solid.Volume - solid.BoundingBox.Volume).Should().BeLessThan(1e-5);
            }
        }
        [Fact]
        public void SurfaceModelBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\surface-model.ifc"))
            {
                var shape = model.Instances.OfType<IfcFaceBasedSurfaceModel>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSurfaceModel(shape, _logger);
                geom.Shells.Count.Should().Be(1);
                ((int)(geom.Shells.First.BoundingBox.Volume)).Should().Be(2000000000);
            }
        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void BrepSolidModelBasicTest(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\brep-model.ifc"))
            {
                var shape = model.Instances.OfType<IfcFacetedBrep>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateSolidSet(shape, _logger).FirstOrDefault();
                geom.Volume.Should().BeApproximately(geom.BoundingBox.Volume, 1e-5);
            }
        }


        [Fact]
        public void MultipleProfileBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\BeamUnitTestsVaryingProfile.ifc"))
            {
                var profiles = model.Instances.OfType<IfcExtrudedAreaSolid>();
                profiles.Count().Should().Be(2);
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                foreach (var profile in profiles)
                {
                    var geom = geomEngine.CreateSolid(profile, _logger);
                    geom.Volume.Should().BeGreaterThan(0);
                }
            }
        }


        #region IfcAdvancedBrep geometries

        [Fact]
        public void AdvancedMultiSegmentPolylineTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\advanced-face-multisegment-polyline.ifc"))
            {
                var shape = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSolid(shape, _logger);
                Math.Abs(geom.Volume - 72767).Should().BeLessThan(1);
            }
        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void BrepSolidModelAdvancedTest(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\cube-advanced-brep.ifc"))
            {
                var shape = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                shape.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateSolid(shape, _logger);
                geom.Volume.Should().BeApproximately(0.83333333333333282, 1e-7);

            }
        }

        /// <summary>
        /// This model has a composite curve with 12 segments, the 11th segment is an incorrectly defined trimed circle, the axis is wrongly defined
        /// The process should fail and handle the failure correctly
        /// </summary>
        [Fact]
        public void can_handle_discontinuous_composite_curve()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\discontinuous_composite_curve.ifc"))
            {
                var cc = model.Instances.OfType<IIfcCompositeCurve>().FirstOrDefault();
                cc.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);

                var exception = Assert.Throws<XbimGeometryFactoryException>(() => geomEngine.CreateWire(cc, _logger));
                exception.Message.Should().Be("IfcCompositeCurve could not be built as a wire");
            }
        }

        [Fact]
        public void AdvancedBrepTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\BasinAdvancedBrep.ifc"))
            {
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                advancedBrep.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var basinSolids = geomEngine.CreateSolidSet(advancedBrep, _logger);
                basinSolids.Sum(s => s.Volume).Should().BeApproximately(2045022.3839364732, 1e-7);
            }
        }
        [Fact]
        public void AdvancedBrepComplexCurvesandSurfacesTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\Axis2PlacementError.ifc"))
            {
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault(i => i.EntityLabel == 27743);
                model.AddRevitWorkArounds();
                //units are not correctly set in the ifc file
                model.ModelFactors.Initialise(1, 1e-3, 1e-2);
                advancedBrep.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var basin = geomEngine.CreateSolidSet(advancedBrep, _logger);
                basin.Count().Should().Be(2);
                basin.Sum(s => s.Volume).Should().BeApproximately(44869362.59648641, 1e-7);

            }
        }


        [Fact]
        public void TriangulatedFaceSetAdvancedTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\BasinTessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                triangulatedFaceSet.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var basin = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                basin.BoundingBox.Volume.Should().BeApproximately(23938449.816244926, 1e-5);

            }
        }



        #endregion

        #region Tessellation tests

        [Fact]
        public void TriangulatedFaceSet1Test()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\beam-straight-i-shape-tessellated.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                triangulatedFaceSet.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                geom.Shells.Count.Should().Be(1); ;
                Math.Abs(geom.Shells.First.BoundingBox.Volume - 1.32).Should().BeLessThan(1e-5);

            }
        }
        //Commented out due to its time taken
        //[Fact]
        //public void TriangulatedFaceSet4Test()
        //{
        //    using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\beam-curved-i-shape-tessellated.ifc"))
        //    {
        //        var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
        //        triangulatedFaceSet);
        //        var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
        //        geom.Shells.Count == 1);
        //        Math.Abs(geom.Shells.First.BoundingBox.Volume - 13.337264) < 1e-5);

        //    }
        //}

        [Fact]
        public void TriangulatedFaceSet2Test()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\column-straight-rectangle-tessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                triangulatedFaceSet.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                geom.Shells.Count.Should().Be(1);
                Math.Abs(geom.Shells.First.BoundingBox.Volume - 7680).Should().BeLessThan(1e-5);

            }
        }
        [Fact]
        public void TriangulatedFaceSet3Test()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\slab-tessellated-unique-vertices.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                triangulatedFaceSet.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                geom.Shells.Count.Should().Be(1);
                Math.Abs(geom.Shells.First.BoundingBox.Volume - 103.92304).Should().BeLessThan(1e-5);

            }
        }
        #endregion

        #region Grid placement

        [Fact]
        public void GridTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\grid-placement.ifc"))
            {

                var placements = model.Instances.OfType<IIfcGridPlacement>();
                placements.Should().NotBeEmpty();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                foreach (var p in placements)
                {
                    XbimMatrix3D m = geomEngine.ToMatrix3D(p, _logger);
                    m.IsIdentity.Should().BeFalse();
                }
                //make a graphic of the grid
                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                ifcGrid.Should().NotBeNull();
                var geom = geomEngine.CreateGrid(ifcGrid, _logger);
                geom.Count.Should().BeGreaterThan(0, "At least one solid must be returned");
                foreach (var solid in geom)
                {
                    solid.Volume.Should().BeGreaterThan(0);
                }
            }
        }

        [Fact]
        public void GridWithIfcLineTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\grid-lines.ifc"))
            {

                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                ifcGrid.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateGrid(ifcGrid, _logger);
                geom.Count.Should().BeGreaterThan(0, "At least one solid must be returned");
                foreach (var solid in geom)
                {
                    solid.Volume.Should().BeGreaterThan(0);
                }
            }
        }


        #endregion

        #region Tapered extrusions

        [Fact]
        public void ExtrudedAreaSolidTaperedTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\air-terminal-element.ifc"))
            {
                var taperedSolid = model.Instances.OfType<IfcExtrudedAreaSolidTapered>().FirstOrDefault();
                taperedSolid.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var bar = geomEngine.CreateSolid(taperedSolid, _logger);
                bar.Volume.Should().BeGreaterThan(0);
            }
        }

        [Fact]
        public void RevolvedAreaSolidTaperedTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\beam-revolved-solid-tapered.ifc"))
            {
                var taperedSolid = model.Instances.OfType<IfcRevolvedAreaSolidTapered>().FirstOrDefault();
                taperedSolid.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var bar = geomEngine.CreateSolid(taperedSolid, _logger);
                bar.Volume.Should().BeGreaterThan(0);
            }
        }





        [Fact]
        public void WireInitFromIfcIndexedPolyCurveTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\WirePolycurve.ifc"))
            {

                var shape = model.Instances[185] as IIfcGeometricRepresentationItem;
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                IXbimGeometryObject geomObject = geomEngine.Create(shape, _logger);
                geomObject.IsValid.Should().BeTrue();

            }
        }

        [Fact]
        public void SectionedSpineTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\sectioned-spine.ifc"))
            {
                var sectionedSpine = model.Instances.OfType<IfcSectionedSpine>().FirstOrDefault();
                sectionedSpine.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var bar = geomEngine.CreateSolid(sectionedSpine, _logger);
                bar.Volume.Should().BeGreaterThan(0);
            }
        }

        //var composite = (IIfcCompositeCurve)model.Instances[75];
        //foreach (var segment in composite.Segments)
        //{
        //    var curve = geomEngine.CreateCurve(segment.ParentCurve);
        //    var wire = geomEngine.CreateWire(segment.ParentCurve);
        //    Math.Abs((curve.Start - wire.Start).Length)<1e-5);
        //    Math.Abs((curve.End - wire.End).Length) < 1e-5);
        //}
        [Fact]
        public void FixedReferenceSweptSolidTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\fixed-reference-sweptarea.ifc"))
            {
                var sectionedSpine = model.Instances.OfType<IfcFixedReferenceSweptAreaSolid>().FirstOrDefault();
                sectionedSpine.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var bar = geomEngine.CreateSolid(sectionedSpine, _logger);
                bar.Volume.Should().BeApproximately(2445.140455567097, 1e-5);
            }
        }



        #endregion

        [Fact]
        public void MirroredProfileDefTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\IfcMirroredProfileDef.ifc"))
            {
                var derived = model.Instances[50] as IIfcDerivedProfileDef; //derived profile, mirrored by transform
                var mirrored = model.Instances[177] as IIfcMirroredProfileDef;//mirrored versio of above
                derived.Should().NotBeNull();
                mirrored.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var dFace = geomEngine.CreateFace(derived, _logger);
                var mFace = geomEngine.CreateFace(mirrored, _logger);
                var brepD = dFace.ToBRep;
                var brepM = mFace.ToBRep;
                var differ = new Diff();
                var diffs = differ.DiffText(brepM, brepD);
                mFace.Normal.X.Should().BeApproximately(dFace.Normal.X, 1e-5);
                mFace.Normal.Y.Should().BeApproximately(dFace.Normal.Y, 1e-5);
                mFace.Normal.Z.Should().BeApproximately(dFace.Normal.Z, 1e-5);
                diffs.Length.Should().Be(3);

            }
        }



        [Fact]
        public void CylindricalSurfaceTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\cylindrical-surface.ifc"))
            {
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet();
                foreach (var brep in model.Instances.OfType<IIfcAdvancedBrep>())
                {

                    var geom = geomEngine.CreateSolid(brep, _logger);

                    foreach (var face in geom.Faces)
                    {
                        face.Area.Should().BeGreaterThan(0);

                    }
                    geom.Volume.Should().BeGreaterThan(0);
                    solids.Add(geom);
                }
                solids.Sum(s => s.Volume).Should().BeApproximately(338122748.71573657, 1e-5);
            }
        }
        [Fact]
        public void NotClosedShellTest()
        {
            using (var er = new EntityRepository<IIfcClosedShell>(nameof(NotClosedShellTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var solid = geomEngine.CreateSolidSet(er.Entity, _logger);

                solid.IsValid.Should().BeTrue();

            }
        }
        /// <summary>
        /// This test checks a composite curve that has trimmed IfcLine segments
        /// </summary>
        [Fact]
        public void CompositeCurveWithIfcLineTest()
        {
            using (var er = new EntityRepository<IIfcCompositeCurve>(nameof(CompositeCurveWithIfcLineTest)))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var curve = geomEngine.CreateWire(er.Entity, _logger);
                curve.Should().NotBeNull();
                curve.IsValid.Should().BeTrue();

                curve.IsPlanar.Should().BeTrue();
                curve.Points.Count().Should().Be(17);
            }
        }

        /// <summary>
        /// This test checks a compsite curve that has incorrect defintions for the sense of its segments
        /// </summary>
        [Fact]
        public void CompositeCurveBadSenseTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\composite-curve.ifc"))
            {
                var comp = model.Instances[3268144] as IIfcCompositeCurve;
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateWire(comp, _logger);
            }
        }
        /// <summary>
        /// This test checks for a composite curve that has a trimmed circle that is not within the tolerance of the model at its connections
        /// </summary>
        [Fact]
        public void CompositeCurveBadPrecisionTest()
        {
            using (var er = new EntityRepository<IIfcCompositeCurve>(nameof(CompositeCurveBadPrecisionTest), 1.0, 1e-5, false))
            {
                er.Entity.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(er.Entity.Model, _loggerFactory);
                var wire = geomEngine.CreateWire(er.Entity, _logger);
                wire.Edges.Count.Should().Be(12);
                var curve = geomEngine.CreateCurve(er.Entity, _logger);
                curve.Length.Should().BeApproximately(wire.Length, 0.999);
            }

        }

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void CompositeCurveEmptySegmentTest(XGeometryEngineVersion engineVersion)
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\composite-curve4.ifc"))
            {
                var eas = model.Instances[3676127] as IIfcExtrudedAreaSolid;
                eas.Should().NotBeNull();
                var geomEngine = factory.CreateGeometryEngine(engineVersion, model, _loggerFactory);
                var geom = geomEngine.CreateSolid(eas, _logger);
                geom.Volume.Should().BeApproximately(11443062570.814053, 1e-5);
            }
        }

        /// <summary>
        /// A Polygonally bounded half space that has a boundary that is not a closed area, the composite curve segments do not join
        /// this is an incorrectly defined half space and therefore it cannot build a valid result.
        /// The solid build should fail elegantly
        /// </summary>
        [Fact]
        public void CompositeCurveSegmentsDoNotCloseTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\composite-curve5.ifc"))
            {

                var pbhs = model.Instances[3942238] as IIfcBooleanClippingResult;
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                //test the faulty entity on its own
                var ex = Assert.Throws<XbimGeometryFactoryException>(()=>geomEngine.ModelService.WireFactory.Build(model.Instances[3942179] as IIfcCompositeCurve));
                ex.Message.Should().Be("IfcCompositeCurve could not be built as a wire");
                //see failure exception comes through the stack
                var exSolid = Assert.Throws<XbimGeometryFactoryException>(() => geomEngine.CreateSolidSet(pbhs, _logger).FirstOrDefault());
                exSolid.Message.Should().Be("IfcCompositeCurve could not be built as a wire");
            }
        }

        [Fact]
        public void BooleanOpeningsTotalSubractionTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\boolean-complete-subtraction.ifc"))
            {
                var ifcWall = model.Instances.OfType<IIfcWall>().FirstOrDefault();
                ifcWall.Should().NotBeNull();
                var ifcOpening = model.Instances.OfType<IIfcOpeningElement>().FirstOrDefault();
                ifcOpening.Should().NotBeNull();

                var opening = model.Instances[1133441] as IIfcExtrudedAreaSolid;
                opening.Should().NotBeNull();
                var wall = model.Instances[1133397] as IIfcExtrudedAreaSolid;
                wall.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                //create it in the right position
                var geomOpening = geomEngine.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)(ifcOpening.ObjectPlacement)).RelativePlacement, _logger) as IXbimSolid;
                geomOpening.Should().NotBeNull();
                geomOpening.Volume.Should().BeGreaterThan(0);
                var geomWall = geomEngine.CreateSolid(wall, _logger);
                geomWall.Volume.Should().BeGreaterThan(0);
                var result = geomWall.Cut(geomOpening, model.ModelFactors.Precision);
                result.Count.Should().Be(0);
            }
        }

        /// <summary>
        /// This test has a void with has zero area, it is a 2 segment self intersection polyline
        /// </summary>
        [Fact]
        public void CloseProfileWithVoidsTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\closed-profile-with-voids.ifc"))
            {

                var eas = model.Instances[23512] as IIfcExtrudedAreaSolid;
                eas.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSolid(eas, _logger);
                geom.Volume.Should().BeApproximately(2278352481546.0352, 1e-5);
            }
        }

        [Fact]
        public void TrimmedEllipseTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\TrimmedEllipseTest.ifc"))
            {

                var eas = model.Instances[272261] as IIfcExtrudedAreaSolid;
                eas.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var geom = geomEngine.CreateSolid(eas, _logger);
                geom.Volume.Should().BeApproximately(18117345688.20311, 1e-5);

            }
        }

        [Fact]
        public void LongRunningBooleanTest()
        {
            using (var model = MemoryModel.OpenRead(@"TestFiles\Ifc4TestFiles\long-running-boolean.ifc"))
            {

                var ifcWall = model.Instances[39] as IIfcExtrudedAreaSolid;
                ifcWall.Should().NotBeNull();
                var geomEngine = new XbimGeometryEngine(model, _loggerFactory);
                var solids = geomEngine.CreateSolidSet();
                foreach (var ifcOpening in model.Instances.OfType<IIfcOpeningElement>())
                {
                    var firstOrDefault = ifcOpening.Representation.Representations.FirstOrDefault();
                    firstOrDefault.Should().NotBeNull();
                    {
                        var opening = firstOrDefault.Items.FirstOrDefault() as IIfcGeometricRepresentationItem;
                        var geomOpening = geomEngine.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)(ifcOpening.ObjectPlacement)).RelativePlacement, _logger) as IXbimSolid;
                        geomOpening.Should().NotBeNull();
                        solids.Add(geomOpening);
                    }

                }
                var wallGeom = geomEngine.CreateSolid(ifcWall, _logger);

                var result = wallGeom.Cut(solids, model.ModelFactors.Precision);
                result.Count.Should().BeGreaterThan(0);

            }
        }
        [Fact]
        public void IfcCenterLineProfileDefTest()
        {

            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var semiCircle = IfcModelBuilder.MakeSemiCircle(m, 20);
                    var cl = IfcModelBuilder.MakeCenterLineProfileDef(m, semiCircle, 5);
                    var geomEngine = new XbimGeometryEngine(m, _loggerFactory);
                    var face = geomEngine.CreateFace(cl, _logger);
                    (face as IXbimFace).Should().NotBeNull();
                    ((IXbimFace)face).IsValid.Should().BeTrue("Invalid face returned");
                }
            }
        }

        [Fact]
        public void IfcSurfaceOfLinearExtrusionTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            using (var txn = m.BeginTransaction("Test"))
            {
                var semiCircle = IfcModelBuilder.MakeSemiCircle(m, 20);
                var def = IfcModelBuilder.MakeArbitraryOpenProfileDef(m, semiCircle);
                var cl = IfcModelBuilder.MakeSurfaceOfLinearExtrusion(m, def, 50, new XbimVector3D(0, 0, 1));
                var geomEngine = new XbimGeometryEngine(m, _loggerFactory);
                var face = geomEngine.CreateFace(cl, _logger);
                (face as IXbimFace).Should().NotBeNull();
                ((IXbimFace)face).IsValid.Should().BeTrue("Invalid face returned");
            }
        }

        [Fact]
        public void IfcSurfaceOfRevolutionTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            using (var txn = m.BeginTransaction("Test"))
            {
                var cc = IfcModelBuilder.MakeRationalBSplineCurveWithKnots(m);
                var def = IfcModelBuilder.MakeArbitraryOpenProfileDef(m, cc);
                var rev = IfcModelBuilder.MakeSurfaceOfRevolution(m, def);
                var geomEngine = new XbimGeometryEngine(m, _loggerFactory);
                var face = geomEngine.CreateFace(rev, _logger);
                (face as IXbimFace).Should().NotBeNull();
                ((IXbimFace)face).IsValid.Should().BeTrue("Invalid face returned");
            }
        }


    }
}
