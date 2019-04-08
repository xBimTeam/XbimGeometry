using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;
using Microsoft.Extensions.Logging;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{

    [TestClass]
    public class Ifc4GeometryTests
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<Ifc4GeometryTests>();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }

        [TestMethod]
        public void CentreLineProfileTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(CentreLineProfileTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcExtrudedAreaSolid found");
                var extrudedSolid = geomEngine.CreateSolid(er.Entity, logger);
                HelperFunctions.IsValidSolid(extrudedSolid);
                
            }
        }

        [TestMethod]
        public void TrimmedCurveWithLargeRadianValueTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(TrimmedCurveWithLargeRadianValueTest), true))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcExtrudedAreaSolid found");
                var extrudedSolid = geomEngine.CreateSolid(er.Entity, logger);
                HelperFunctions.IsValidSolid(extrudedSolid);
                Assert.AreEqual<long>((long)extrudedSolid.Volume , 14999524619);
            }
        }


        [TestMethod]
        public void ExtrudedSolidWithNullPositionTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(ExtrudedSolidWithNullPositionTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcExtrudedAreaSolid found");
                var extrudedSolid = geomEngine.CreateSolid(er.Entity, logger);
                HelperFunctions.IsValidSolid(extrudedSolid);
                
            }
        }
        [TestMethod]
        public void FacetedBrepIsValidSolidTest()
        {
            using (var er = new EntityRepository<IIfcFacetedBrep>(nameof(FacetedBrepIsValidSolidTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcFacetedBrep found");
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                HelperFunctions.IsValidSolid(solid);

            }

        }

        [TestMethod]
        public void FacetedBrepWithFacesOutsideNorlamTolerancesTest()
        {
            using (var er = new EntityRepository<IIfcFacetedBrep>(nameof(FacetedBrepWithFacesOutsideNorlamTolerancesTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcFacetedBrep found");
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                HelperFunctions.IsValidSolid(solid);

            }

        }
        [TestMethod]
        public void CsgSolidIsValidSolidTest()
        {
            using (var er = new EntityRepository<IIfcCsgSolid>(nameof(CsgSolidIsValidSolidTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcCsgSolid found");
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                HelperFunctions.IsValidSolid(solid);

            }
        }
        [TestMethod]
        public void IndexedPolyCurveTest()
        {
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>(nameof(IndexedPolyCurveTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcExtrudedAreaSolid found");
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                HelperFunctions.IsValidSolid(solid);

            }
        }

        [TestMethod]
        public void TriangulatedFaceSetBasicTest()
        {
            using (var er = new EntityRepository<IIfcTriangulatedFaceSet>(nameof(TriangulatedFaceSetBasicTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcTriangulatedFaceSet found");
                var beam = geomEngine.CreateSurfaceModel(er.Entity);
                Assert.IsTrue(Math.Abs(beam.BoundingBox.Volume - 20000000) < 1);
            }
        }

        [TestMethod]
        public void CsgSolidBoundingBoxTest()
        {
            using (var er = new EntityRepository<IfcCsgSolid>(nameof(CsgSolidBoundingBoxTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcCsgSolid found");
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                Assert.IsTrue(Math.Abs(solid.Volume - solid.BoundingBox.Volume) < 1e-5);

            }          
        }

        [TestMethod]
        public void ExtrudedAreaSolidBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\extruded-solid.ifc"))
            {
                var eas = model.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(eas);
                var solid = geomEngine.CreateSolid(eas);
                Assert.IsTrue(Math.Abs(solid.Volume - solid.BoundingBox.Volume )< 1e-5);

            }
        }
        [TestMethod]
        public void SurfaceModelBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\surface-model.ifc"))
            {
                var shape = model.Instances.OfType<IfcFaceBasedSurfaceModel>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = geomEngine.CreateSurfaceModel(shape);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue((int)geom.Shells.First.BoundingBox.Volume == 2000000000);
                

            }
        }

        [TestMethod]
        public void BrepSolidModelBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\brep-model.ifc"))
            {
                var shape = model.Instances.OfType<IfcFacetedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = geomEngine.CreateSolidSet(shape).FirstOrDefault();
                Assert.IsTrue(Math.Abs(geom.Volume - geom.BoundingBox.Volume) < 1e-5);
            }
        }

        [TestMethod]
        public void MultipleProfileBasicTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\BeamUnitTestsVaryingProfile.ifc"))
            {
                var profiles = model.Instances.OfType<IfcExtrudedAreaSolid>();
                Assert.IsTrue(profiles.Count()==2);
                foreach (var profile in profiles)
                {
                    var geom = geomEngine.CreateSolid(profile);
                    Assert.IsTrue(geom.Volume >0);
                }
                
            }
        }


        #region IfcAdvancedBrep geometries

        [TestMethod]
        public void AdvancedMultiSegmentPolylineTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\advanced-face-multisegment-polyline.ifc"))
            {
                var shape = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = geomEngine.CreateSolid(shape);
                Assert.IsTrue(Math.Abs(geom.Volume -72765) < 1);
            }
        }
        
        [TestMethod]
        public void BrepSolidModelAdvancedTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\cube-advanced-brep.ifc"))
            {
                var shape = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = geomEngine.CreateSolid(shape);
                Assert.IsTrue(Math.Abs(geom.Volume - 0.83333333) < 1e-5);
            }
        }

        [TestMethod]
        public void AdvancedBrepTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\BasinAdvancedBrep.ifc"))
            {
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(advancedBrep);
                var basin = geomEngine.CreateSolid(advancedBrep);
                Assert.IsTrue((int)basin.Volume == 2045022);
            }
        }
        [TestMethod]
        public void AdvancedBrepComplexCurvesandSurfacesTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\Axis2PlacementError.ifc"))
            {
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault(i => i.EntityLabel== 27743);
                bool wa = model.ModelFactors.ApplyWorkAround("#SurfaceOfLinearExtrusion");
                Assert.IsNotNull(advancedBrep);
                var basin = geomEngine.CreateSolid(advancedBrep);
                Assert.IsTrue((int)basin.Volume == 44025929);
            }
        }
        [TestMethod]
        public void TriangulatedFaceSetAdvancedTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\BasinTessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var basin = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue((int)basin.BoundingBox.Volume == 23960321); 

            }
        }

        [TestMethod]
        public void AdvancedSweptSolidTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\ReinforcingBar.ifc"))
            {
                var advancedSweep = model.Instances.OfType<IfcSweptDiskSolid>().FirstOrDefault();
                Assert.IsNotNull(advancedSweep);
                var bar = geomEngine.CreateSolid(advancedSweep);
                Assert.IsTrue((int)bar.Volume == 131934);
            }
        }


        #endregion

        #region Tessellation tests

        [TestMethod]
        public void TriangulatedFaceSet1Test()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\beam-straight-i-shape-tessellated.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume-1.32) <1e-5);

            }
        }
        //Commented out due to its time taken
        //[TestMethod]
        //public void TriangulatedFaceSet4Test()
        //{
        //    using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\beam-curved-i-shape-tessellated.ifc"))
        //    {
        //        var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
        //        Assert.IsNotNull(triangulatedFaceSet);
        //        var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
        //        Assert.IsTrue(geom.Shells.Count == 1);
        //        Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 13.337264) < 1e-5);

        //    }
        //}

        [TestMethod]
        public void TriangulatedFaceSet2Test()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\column-straight-rectangle-tessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 7680) < 1e-5);

            }
        }
        [TestMethod]
        public void TriangulatedFaceSet3Test()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\slab-tessellated-unique-vertices.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = geomEngine.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 103.92304) < 1e-5);

            }
        }
        #endregion

        #region Grid placement

        [TestMethod]
        public void GridTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\grid-placement.ifc"))
            {
                   
                var placements = model.Instances.OfType<IIfcGridPlacement>();
                Assert.IsTrue(placements.Any());
                foreach (var p in placements)
                {
                    XbimMatrix3D m = geomEngine.ToMatrix3D(p);
                    Assert.IsFalse(m.IsIdentity);
                }
                //make a graphic of the grid
                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                Assert.IsNotNull(ifcGrid);           
                var geom = geomEngine.CreateGrid(ifcGrid);
                Assert.IsTrue(geom.Count > 0, "At least one solid must be returned");
                foreach (var solid in geom)
                {
                    Assert.IsTrue(solid.Volume>0);
                }
            }
        }

        [TestMethod]
        public void GridWithIfcLineTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\grid-lines.ifc"))
            {

                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                Assert.IsNotNull(ifcGrid);
                var geom = geomEngine.CreateGrid(ifcGrid);
                Assert.IsTrue(geom.Count > 0, "At least one solid must be returned");
                foreach (var solid in geom)
                {
                    Assert.IsTrue(solid.Volume > 0);
                }
            }
        }


        #endregion

        #region Tapered extrusions

        [TestMethod]
        public void ExtrudedAreaSolidTaperedTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\air-terminal-element.ifc"))
            {
                var taperedSolid = model.Instances.OfType<IfcExtrudedAreaSolidTapered>().FirstOrDefault();
                Assert.IsNotNull(taperedSolid);
                var bar = geomEngine.CreateSolid(taperedSolid, logger);
                Assert.IsTrue((int)bar.Volume>0);
            }
        }

        [TestMethod]
        public void RevolvedAreaSolidTaperedTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\beam-revolved-solid-tapered.ifc"))
            {
                var taperedSolid = model.Instances.OfType<IfcRevolvedAreaSolidTapered>().FirstOrDefault();
                Assert.IsNotNull(taperedSolid);
                var bar = geomEngine.CreateSolid(taperedSolid);
                Assert.IsTrue(bar.Volume>0);
            }
        }
        [TestMethod]
        public void SweptDiskSolidPolygonalTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\swept-disk-solid-polygonal.ifc"))
            {
                var sweptPolygonalSolid = model.Instances.OfType<IfcSweptDiskSolidPolygonal>().FirstOrDefault();
                Assert.IsNotNull(sweptPolygonalSolid);
                var bar = geomEngine.CreateSolid(sweptPolygonalSolid);
                Assert.IsTrue(bar.Volume > 0);
            }
        }
        [TestMethod]
        public void WireInitFromIfcIndexedPolyCurveTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\WirePolycurve.ifc"))
            {
              
                    var shape = model.Instances[185] as IIfcGeometricRepresentationItem;
                    IXbimGeometryObject geomObject = geomEngine.Create(shape);
                    Assert.IsTrue(geomObject.IsValid);
                
            }
        }

        [TestMethod]
        public void SectionedSpineTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\sectioned-spine.ifc"))
            {
                var sectionedSpine = model.Instances.OfType<IfcSectionedSpine>().FirstOrDefault();
                Assert.IsNotNull(sectionedSpine);
                var bar = geomEngine.CreateSolid(sectionedSpine);
                Assert.IsTrue(bar.Volume > 0);
            }
        }

 //var composite = (IIfcCompositeCurve)model.Instances[75];
                //foreach (var segment in composite.Segments)
                //{
                //    var curve = geomEngine.CreateCurve(segment.ParentCurve);
                //    var wire = geomEngine.CreateWire(segment.ParentCurve);
                //    Assert.IsTrue(Math.Abs((curve.Start - wire.Start).Length)<1e-5);
                //    Assert.IsTrue(Math.Abs((curve.End - wire.End).Length) < 1e-5);
                //}
        [TestMethod]
        public void FixedReferenceSweptSolidTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\fixed-reference-sweptarea.ifc", logger))
            {
                var sectionedSpine = model.Instances.OfType<IfcFixedReferenceSweptAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(sectionedSpine);
                var bar = geomEngine.CreateSolid(sectionedSpine);
                Assert.IsTrue(bar.Volume > 0);
            }
        }
        
        [TestMethod]
        public void SurfaceCurveSweptAreaSolidTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\surface-curve-swept-area.ifc"))
            {
                var surfaceSweep = model.Instances.OfType<IfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(surfaceSweep);
                IIfcSurfaceOfLinearExtrusion le = (IIfcSurfaceOfLinearExtrusion)surfaceSweep.ReferenceSurface;
                XbimVector3D v = le.ExtrusionAxis;

                var bar = geomEngine.CreateSolid(surfaceSweep);
                Assert.IsTrue(bar.Volume > 0);
            }
        }                                     
        
        #endregion

        [TestMethod]
        public void MirroredProfileDefTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\IfcMirroredProfileDef.ifc"))
            {
                var derived = model.Instances[50] as IIfcDerivedProfileDef; //derived profile, mirrored by transform
                var mirrored = model.Instances[177] as IIfcMirroredProfileDef;//mirrored versio of above
                Assert.IsNotNull(derived);
                Assert.IsNotNull(mirrored);
                
                var dFace = geomEngine.CreateFace(derived);
                var mFace = geomEngine.CreateFace(mirrored);
                var brepD = dFace.ToBRep;
                var brepM = mFace.ToBRep;
                var differ = new Diff();
                var diffs = differ.DiffText(brepM, brepD);
                Assert.IsTrue(mFace.Normal==dFace.Normal);
                Assert.IsTrue(diffs.Length==3);
               
            }
        }



        [TestMethod]
        public void CylindricalSurfaceTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\cylindrical-surface.ifc"))
            {
                foreach (var brep in model.Instances.OfType<IIfcAdvancedBrep>())
                {
                    var geom = geomEngine.CreateSolid(brep);
                    foreach (var face in geom.Faces)
                    {
                        Assert.IsTrue(face.Area > 0);
                    }
                    Assert.IsTrue(geom.Volume > 0);
                }
            }
        }
        [TestMethod]
        public void NotClosedShellTest()
        {
            using (var er = new EntityRepository<IIfcClosedShell>(nameof(NotClosedShellTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcClosedShell found");

                var solid = geomEngine.CreateSolidSet(er.Entity, logger);

                Assert.IsTrue(solid.IsValid);
               
            }
        }
        /// <summary>
        /// This test checks a composite curve that has trimmed IfcLine segments
        /// </summary>
        [TestMethod]
        public void CompositeCurveWithIfcLineTest()
        {
            using (var er = new EntityRepository<IIfcCompositeCurve>(nameof(CompositeCurveWithIfcLineTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcCompositeCurve found");

                var curve = geomEngine.CreateWire(er.Entity, logger);
                Assert.IsNotNull(curve);
                Assert.IsTrue(curve.IsValid);
                Assert.IsTrue(curve.IsClosed);
                Assert.IsTrue(curve.IsPlanar);
                Assert.IsTrue(curve.Points.Count()==16);
            }
        }

        /// <summary>
        /// This test checks a compsite curve that has incorrect defintions for the sense of its segments
        /// </summary>
        [TestMethod]
        public void CompositeCurveBadSenseTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\composite-curve.ifc"))
            {
                var comp = model.Instances[3268144] as IIfcCompositeCurve;
                var geom = geomEngine.CreateWire(comp);
            }
        }
        /// <summary>
        /// This test checks for a composite curve that has a trimmed circle that is not within the tolerance of the model at its connections
        /// </summary>
        [TestMethod]
        public void CompositeCurveBadPrecisionTest()
        {
            using (var er = new EntityRepository<IIfcCompositeCurve>(nameof(CompositeCurveBadPrecisionTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcCompositeProfileDef found");
                var wire = geomEngine.CreateWire(er.Entity, logger);
                Assert.IsTrue(wire.Edges.Count == 12, "This wire should have 12 edges");
            }
            
        }

        [TestMethod]
        public void CompositeCurveEmptySegmentTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\composite-curve4.ifc"))
            {
                var eas = model.Instances[3676127] as IIfcExtrudedAreaSolid;
                Assert.IsNotNull(eas);
                var geom = geomEngine.CreateSolid(eas);
                Assert.IsTrue((geom.Volume > 0));
            }
        }

        [TestMethod]
        public void CompositeCurveSegmentsDoNotCloseTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\composite-curve5.ifc"))
            {
               
                    //try building the polygonally bounded half space that has the faulty curve, which is now a seam
                    var pbhs = model.Instances[3942238] as IIfcBooleanClippingResult;
                    var solid = geomEngine.CreateSolidSet(pbhs).FirstOrDefault();
                    Assert.IsTrue(solid.Volume > 0);
                   
            }
        }

        [TestMethod]
        public void BooleanOpeningsTotalSubractionTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\boolean-complete-subtraction.ifc"))
            {
                var ifcWall = model.Instances.OfType<IIfcWall>().FirstOrDefault();
                Assert.IsNotNull(ifcWall);
                var ifcOpening = model.Instances.OfType<IIfcOpeningElement>().FirstOrDefault();
                Assert.IsNotNull(ifcOpening);

                var opening = model.Instances[1133441] as IIfcExtrudedAreaSolid;
                Assert.IsNotNull(opening);
                var wall = model.Instances[1133397] as IIfcExtrudedAreaSolid;
                Assert.IsNotNull(wall);
                //create it in the right position
                var geomOpening = geomEngine.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)(ifcOpening.ObjectPlacement)).RelativePlacement) as IXbimSolid;
                Assert.IsNotNull(geomOpening);
                Assert.IsTrue((geomOpening.Volume > 0));
                var geomWall = geomEngine.CreateSolid(wall);
                Assert.IsTrue((geomWall.Volume > 0));
                var result = geomWall.Cut(geomOpening, model.ModelFactors.Precision);
                Assert.IsTrue(result.Count==0);
            }
        }

        [TestMethod]
        public void CloseProfileWithVoidsTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\closed-profile-with-voids.ifc"))
            {
               
                    var eas = model.Instances[23512] as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(eas);
                    var geom = geomEngine.CreateSolid(eas);
                    Assert.IsTrue((geom.Volume > 0));
                    
                
            }
        }

        [TestMethod]
        public void TrimmedEllipseTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\TrimmedEllipseTest.ifc"))
            {
               
                    var eas = model.Instances[272261] as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(eas);
                    var geom = geomEngine.CreateSolid(eas);
                    Assert.IsTrue((geom.Volume > 0));   
                
            }
        }

        [TestMethod]
        public void LongRunningBooleanTest()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\Ifc4TestFiles\long-running-boolean.ifc"))
            {
                
                    var ifcWall = model.Instances[39] as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(ifcWall);
                    var solids = geomEngine.CreateSolidSet();
                    foreach (var ifcOpening in model.Instances.OfType<IIfcOpeningElement>())
                    {
                        var firstOrDefault = ifcOpening.Representation.Representations.FirstOrDefault();
                        Assert.IsNotNull(firstOrDefault);
                        {
                            var opening = firstOrDefault.Items.FirstOrDefault() as IIfcGeometricRepresentationItem;
                            var geomOpening = geomEngine.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)(ifcOpening.ObjectPlacement)).RelativePlacement) as IXbimSolid;
                            Assert.IsNotNull(geomOpening);
                            solids.Add(geomOpening);
                        }

                    }
                    var wallGeom = geomEngine.CreateSolid(ifcWall);

                    var result = wallGeom.Cut(solids, model.ModelFactors.Precision);
                    Assert.IsTrue(result.Count >0);
                
            }
        }
        [TestMethod]
        public void IfcCenterLineProfileDefTest()
        {

            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var semiCircle = IfcModelBuilder.MakeSemiCircle(m, 20);
                    var cl = IfcModelBuilder.MakeCenterLineProfileDef(m, semiCircle, 5);
                    var face = geomEngine.CreateFace(cl);
                    Assert.IsNotNull(face as IXbimFace, "Wrong type returned");
                    Assert.IsTrue(((IXbimFace)face).IsValid, "Invalid face returned");
                }
            }
        }

        [TestMethod]
        public void IfcSurfaceOfLinearExtrusionTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var semiCircle = IfcModelBuilder.MakeSemiCircle(m, 20);
                    var def = IfcModelBuilder.MakeArbitraryOpenProfileDef(m, semiCircle);
                    var cl = IfcModelBuilder.MakeSurfaceOfLinearExtrusion(m, def, 50, new XbimVector3D(0, 0, 1));
                    var face = geomEngine.CreateFace(cl);
                    Assert.IsNotNull(face as IXbimFace, "Wrong type returned");
                    Assert.IsTrue(((IXbimFace)face).IsValid, "Invalid face returned");
                }
            }
        }

        [TestMethod]
        public void IfcSurfaceOfRevolutionTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var cc = IfcModelBuilder.MakeRationalBSplineCurveWithKnots(m);
                    var def = IfcModelBuilder.MakeArbitraryOpenProfileDef(m, cc);
                    var rev = IfcModelBuilder.MakeSurfaceOfRevolution(m, def);
                    var face = geomEngine.CreateFace(rev);
                    Assert.IsNotNull(face as IXbimFace, "Wrong type returned");
                    Assert.IsTrue(((IXbimFace)face).IsValid, "Invalid face returned");
                }
            }
        }


    }
}
