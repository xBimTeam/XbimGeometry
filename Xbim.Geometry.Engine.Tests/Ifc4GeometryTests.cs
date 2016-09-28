using System;
using System.Linq;
using GeometryTests;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Logging;
using Xbim.Ifc;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;


namespace Ifc4GeometryTests
{
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [DeploymentItem(@"Ifc4TestFiles\", "Ifc4TestFiles")]
    [TestClass]
    public class Ifc4GeometryTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
       


        [TestMethod]
        public void ExtrudedSolidWithNullPositionTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\Wall.ifc"))
            {
                var extSolid = model.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(extSolid);
                var wall = _xbimGeometryCreator.CreateSolid(extSolid);
                Assert.IsTrue(wall.Volume > 0);
                
            }
        }
        [TestMethod]
        public void BasinBRepTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BasinBRep.ifc"))
            {
                var brep = model.Instances.OfType<IfcFacetedBrep>().FirstOrDefault();
                Assert.IsNotNull(brep);
                var basin = _xbimGeometryCreator.CreateSolid(brep);
                Assert.IsTrue(basin.Volume > 0);

            }
        }
        [TestMethod]
        public void CsgSolidTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\Bath.ifc"))
            {
                var csgSolid = model.Instances.OfType<IfcCsgSolid>().FirstOrDefault();
                Assert.IsNotNull(csgSolid);
                var bath = _xbimGeometryCreator.CreateSolid(csgSolid);
                Assert.IsTrue(bath.Volume > 0);

            }
        }
        [TestMethod]
        public void IndexedPolyCurveTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BeamExtruded.ifc"))
            {
                var extSolid = model.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(extSolid);
                var wall = _xbimGeometryCreator.CreateSolid(extSolid);
                Assert.IsTrue(wall.Volume > 0);

            }
        }

        [TestMethod]
        public void TriangulatedFaceSetBasicTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BeamTessellated.ifc"))
            {
                var faceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(faceSet);
                var beam = _xbimGeometryCreator.CreateSurfaceModel(faceSet);
                Assert.IsTrue(Math.Abs(beam.BoundingBox.Volume - 20000000) <1);

            }
        }

        [TestMethod]
        public void CsgSolidBasicTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\csg-primitive.ifc"))
            {
                var csg = model.Instances.OfType<IfcCsgSolid>().FirstOrDefault();
                Assert.IsNotNull(csg);
                var block = _xbimGeometryCreator.CreateSolid(csg);
                Assert.IsTrue(Math.Abs(block.Volume-block.BoundingBox.Volume) < 1e-5 );

            }
        }
        [TestMethod]
        public void ExtrudedAreaSolidBasicTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\extruded-solid.ifc"))
            {
                var eas = model.Instances.OfType<IfcExtrudedAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(eas);
                var solid = _xbimGeometryCreator.CreateSolid(eas);
                Assert.IsTrue(Math.Abs(solid.Volume - solid.BoundingBox.Volume )< 1e-5);

            }
        }
        [TestMethod]
        public void SurfaceModelBasicTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\surface-model.ifc"))
            {
                var shape = model.Instances.OfType<IfcFaceBasedSurfaceModel>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = _xbimGeometryCreator.CreateSurfaceModel(shape);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue((int)geom.Shells.First.BoundingBox.Volume == 2000000000);
                

            }
        }

        [TestMethod]
        public void BrepSolidModelBasicTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\brep-model.ifc"))
            {
                var shape = model.Instances.OfType<IfcFacetedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = _xbimGeometryCreator.CreateSolid(shape);
                Assert.IsTrue(Math.Abs(geom.Volume - geom.BoundingBox.Volume) < 1e-5);
            }
        }

        [TestMethod]
        public void MultipleProfileBasicTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BeamUnitTestsVaryingProfile.ifc"))
            {
                var profiles = model.Instances.OfType<IfcExtrudedAreaSolid>();
                Assert.IsTrue(profiles.Count()==2);
                foreach (var profile in profiles)
                {
                    var geom = _xbimGeometryCreator.CreateSolid(profile);
                    Assert.IsTrue(geom.Volume >0);
                }
                
            }
        }


        #region IfcAdvancedBrep geometries

        [TestMethod]
        public void AdvancedMultiSegmentPolylineTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\advanced-face-multisegment-polyline.ifc"))
            {
                var shape = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = _xbimGeometryCreator.CreateSolid(shape);
                Assert.IsTrue(Math.Abs(geom.Volume -72765) < 1);
            }
        }
        
        [TestMethod]
        public void BrepSolidModelAdvancedTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\cube-advanced-brep.ifc"))
            {
                var shape = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(shape);
                var geom = _xbimGeometryCreator.CreateSolid(shape);
                Assert.IsTrue(Math.Abs(geom.Volume - 0.83333333) < 1e-5);
            }
        }

        [TestMethod]
        public void AdvancedBrepTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BasinAdvancedBrep.ifc"))
            {
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault();
                Assert.IsNotNull(advancedBrep);
                var basin = _xbimGeometryCreator.CreateSolid(advancedBrep);
                Assert.IsTrue((int)basin.Volume == 2045022);
            }
        }
        [TestMethod]
        public void AdvancedBrepComplexCurvesandSurfacesTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\Axis2PlacementError.ifc"))
            {
                var advancedBrep = model.Instances.OfType<IfcAdvancedBrep>().FirstOrDefault(i => i.EntityLabel== 27743);
                bool wa = model.ModelFactors.ApplyWorkAround("#SurfaceOfLinearExtrusion");
                Assert.IsNotNull(advancedBrep);
                var basin = _xbimGeometryCreator.CreateSolid(advancedBrep);
                Assert.IsTrue((int)basin.Volume == 44025929);
            }
        }
        [TestMethod]
        public void TriangulatedFaceSetAdvancedTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\BasinTessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var basin = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue((int)basin.BoundingBox.Volume == 23913891);

            }
        }

        [TestMethod]
        public void AdvancedSweptSolidTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\ReinforcingBar.ifc"))
            {
                var advancedSweep = model.Instances.OfType<IfcSweptDiskSolid>().FirstOrDefault();
                Assert.IsNotNull(advancedSweep);
                var bar = _xbimGeometryCreator.CreateSolid(advancedSweep);
                Assert.IsTrue((int)bar.Volume == 131934);
            }
        }


        #endregion

        #region Tessellation tests

        [TestMethod]
        public void TriangulatedFaceSet1Test()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\beam-straight-i-shape-tessellated.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume-1.32) <1e-5);

            }
        }
        //Commented out due to its time taken
        //[TestMethod]
        //public void TriangulatedFaceSet4Test()
        //{
        //    using (var model = IfcStore.Open(@"Ifc4TestFiles\beam-curved-i-shape-tessellated.ifc"))
        //    {
        //        var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
        //        Assert.IsNotNull(triangulatedFaceSet);
        //        var geom = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
        //        Assert.IsTrue(geom.Shells.Count == 1);
        //        Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 13.337264) < 1e-5);

        //    }
        //}

        [TestMethod]
        public void TriangulatedFaceSet2Test()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\column-straight-rectangle-tessellation.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 7680) < 1e-5);

            }
        }
        [TestMethod]
        public void TriangulatedFaceSet3Test()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\slab-tessellated-unique-vertices.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 103.92304) < 1e-5);

            }
        }
        #endregion

        #region Grid placement

        [TestMethod]
        public void GridTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\grid-placement.ifc"))
            {
                   
                var placements = model.Instances.OfType<IIfcGridPlacement>();
                Assert.IsTrue(placements.Any());
                foreach (var p in placements)
                {
                    XbimMatrix3D m = _xbimGeometryCreator.ToMatrix3D(p);
                    Assert.IsFalse(m.IsIdentity);
                }
                //make a graphic of the grid
                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                Assert.IsNotNull(ifcGrid);
                var geom = _xbimGeometryCreator.CreateGrid(ifcGrid);
                foreach (var solid in geom)
                {
                    Assert.IsTrue(solid.Volume>0);
                }
            }
        }

        [TestMethod]
        public void GridWithIfcLineTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\grid-lines.ifc"))
            {

                var ifcGrid = model.Instances.OfType<IIfcGrid>().FirstOrDefault();
                Assert.IsNotNull(ifcGrid);
                var geom = _xbimGeometryCreator.CreateGrid(ifcGrid);
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
            using (var model = IfcStore.Open(@"Ifc4TestFiles\air-terminal-element.ifc"))
            {
                var taperedSolid = model.Instances.OfType<IfcExtrudedAreaSolidTapered>().FirstOrDefault();
                Assert.IsNotNull(taperedSolid);
                var bar = _xbimGeometryCreator.CreateSolid(taperedSolid);
                Assert.IsTrue((int)bar.Volume>0);
            }
        }

        [TestMethod]
        public void RevolvedAreaSolidTaperedTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\beam-revolved-solid-tapered.ifc"))
            {
                var taperedSolid = model.Instances.OfType<IfcRevolvedAreaSolidTapered>().FirstOrDefault();
                Assert.IsNotNull(taperedSolid);
                var bar = _xbimGeometryCreator.CreateSolid(taperedSolid);
                Assert.IsTrue(bar.Volume>0);
            }
        }
        [TestMethod]
        public void SweptDiskSolidPolygonalTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\swept-disk-solid-polygonal.ifc"))
            {
                var sweptPolygonalSolid = model.Instances.OfType<IfcSweptDiskSolidPolygonal>().FirstOrDefault();
                Assert.IsNotNull(sweptPolygonalSolid);
                var bar = _xbimGeometryCreator.CreateSolid(sweptPolygonalSolid);
                Assert.IsTrue(bar.Volume > 0);
            }
        }

        [TestMethod]
        public void SectionedSpineTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\sectioned-spine.ifc"))
            {
                var sectionedSpine = model.Instances.OfType<IfcSectionedSpine>().FirstOrDefault();
                Assert.IsNotNull(sectionedSpine);
                var bar = _xbimGeometryCreator.CreateSolid(sectionedSpine);
                Assert.IsTrue(bar.Volume > 0);
            }
        }

 //var composite = (IIfcCompositeCurve)model.Instances[75];
                //foreach (var segment in composite.Segments)
                //{
                //    var curve = _xbimGeometryCreator.CreateCurve(segment.ParentCurve);
                //    var wire = _xbimGeometryCreator.CreateWire(segment.ParentCurve);
                //    Assert.IsTrue(Math.Abs((curve.Start - wire.Start).Length)<1e-5);
                //    Assert.IsTrue(Math.Abs((curve.End - wire.End).Length) < 1e-5);
                //}
        [TestMethod]
        public void FixedReferenceSweptSolidTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\fixed-reference-swept-area.ifc"))
            {
                var sectionedSpine = model.Instances.OfType<IfcFixedReferenceSweptAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(sectionedSpine);
                var bar = _xbimGeometryCreator.CreateSolid(sectionedSpine);
                Assert.IsTrue(bar.Volume > 0);
            }
        }
        
        [TestMethod]
        public void SurfaceCurveSweptAreaSolidTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\surface-curve-swept-area.ifc"))
            {
                var surfaceSweep = model.Instances.OfType<IfcSurfaceCurveSweptAreaSolid>().FirstOrDefault();
                Assert.IsNotNull(surfaceSweep);
                IIfcSurfaceOfLinearExtrusion le = (IIfcSurfaceOfLinearExtrusion)surfaceSweep.ReferenceSurface;
                XbimVector3D v = le.ExtrusionAxis;

                var bar = _xbimGeometryCreator.CreateSolid(surfaceSweep);
                Assert.IsTrue(bar.Volume > 0);
            }
        }                                     
        
        #endregion

        [TestMethod]
        public void MirroredProfileDefTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\IfcMirroredProfileDef.ifc"))
            {
                var derived = model.Instances[50] as IIfcDerivedProfileDef; //derived profile, mirrored by transform
                var mirrored = model.Instances[177] as IIfcMirroredProfileDef;//mirrored versio of above
                Assert.IsNotNull(derived);
                Assert.IsNotNull(mirrored);
                
                var dFace = _xbimGeometryCreator.CreateFace(derived);
                var mFace = _xbimGeometryCreator.CreateFace(mirrored);
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
            using (var model = IfcStore.Open(@"Ifc4TestFiles\cylindrical-surface.ifc"))
            {
                foreach (var brep in model.Instances.OfType<IIfcAdvancedBrep>())
                {
                    var geom = _xbimGeometryCreator.CreateSolid(brep);
                    foreach (var face in geom.Faces)
                    {
                        Assert.IsTrue(face.Area > 0);
                    }
                    Assert.IsTrue(geom.Volume > 0);
                }
            }
        }

        /// <summary>
        /// This test checks a compsite curve that has incorrect defintions for the sense of its segments
        /// </summary>
        [TestMethod]
        public void CompositeCurveBadSenseTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\composite-curve.ifc"))
            {
                var comp = model.Instances[3268144] as IIfcCompositeCurve;
                var geom = _xbimGeometryCreator.CreateWire(comp);
            }
        }
        /// <summary>
        /// This test checks for a composite curve that has a trimmed circle that is not within the tolerance of the model at its connections
        /// </summary>
        [TestMethod]
        public void CompositeCurveBadPrecisionTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\composite-curve2.ifc"))
            {
                var eas = model.Instances[3205] as IIfcExtrudedAreaSolid;
                Assert.IsNotNull(eas);
                var geom = _xbimGeometryCreator.CreateSolid(eas);
                Assert.IsTrue((geom.Volume>0));
            }
        }

        [TestMethod]
        public void CompositeCurveEmptySegmentTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\composite-curve4.ifc"))
            {
                var eas = model.Instances[3676127] as IIfcExtrudedAreaSolid;
                Assert.IsNotNull(eas);
                var geom = _xbimGeometryCreator.CreateSolid(eas);
                Assert.IsTrue((geom.Volume > 0));
            }
        }

        [TestMethod]
        public void CompositeCurveSegmentsDoNotCloseTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\composite-curve5.ifc"))
            {
                using (var eventTrace = LoggerFactory.CreateEventTrace())
                {
                    //try building the polygonally bounded half space that has the faulty curve, which is now a seam
                    var pbhs = model.Instances[3942238] as IIfcBooleanClippingResult;
                    var solid = _xbimGeometryCreator.CreateSolid(pbhs);
                    Assert.IsTrue(solid.Volume > 0);
                    Assert.IsTrue(eventTrace.Events.Count == 2); //2 events should have been raised from this call
                }
            }
        }

        [TestMethod]
        public void BooleanOpeningsTotalSubractionTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\boolean-complete-subtraction.ifc"))
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
                var geomOpening = _xbimGeometryCreator.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)(ifcOpening.ObjectPlacement)).RelativePlacement) as IXbimSolid;
                Assert.IsNotNull(geomOpening);
                Assert.IsTrue((geomOpening.Volume > 0));
                var geomWall = _xbimGeometryCreator.CreateSolid(wall);
                Assert.IsTrue((geomWall.Volume > 0));
                var result = geomWall.Cut(geomOpening, model.ModelFactors.Precision);
                Assert.IsTrue(result.Count==0);
            }
        }

        [TestMethod]
        public void CloseProfileWithVoidsTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\closed-profile-with-voids.ifc"))
            {
                using (var eventTrace = LoggerFactory.CreateEventTrace())
                {
                    var eas = model.Instances[23512] as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(eas);
                    var geom = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue((geom.Volume > 0));
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                }
            }
        }

        [TestMethod]
        public void TrimmedEllipseTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\TrimmedEllipseTest.ifc"))
            {
                using (var eventTrace = LoggerFactory.CreateEventTrace())
                {
                    var eas = model.Instances[272261] as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(eas);
                    var geom = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue((geom.Volume > 0));
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                }
            }
        }

        [TestMethod]
        public void LongRunningBooleanTest()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\long-running-boolean.ifc"))
            {
                using (var eventTrace = LoggerFactory.CreateEventTrace())
                {
                    var ifcWall = model.Instances[39] as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(ifcWall);
                    var solids = _xbimGeometryCreator.CreateSolidSet();
                    foreach (var ifcOpening in model.Instances.OfType<IIfcOpeningElement>())
                    {
                        var firstOrDefault = ifcOpening.Representation.Representations.FirstOrDefault();
                        Assert.IsNotNull(firstOrDefault);
                        {
                            var opening = firstOrDefault.Items.FirstOrDefault() as IIfcGeometricRepresentationItem;
                            var geomOpening = _xbimGeometryCreator.Create(opening, (IIfcAxis2Placement3D)((IIfcLocalPlacement)(ifcOpening.ObjectPlacement)).RelativePlacement) as IXbimSolid;
                            Assert.IsNotNull(geomOpening);
                            solids.Add(geomOpening);
                        }

                    }
                    var wallGeom = _xbimGeometryCreator.CreateSolid(ifcWall);

                    var result = wallGeom.Cut(solids, model.ModelFactors.Precision);
                    Assert.IsTrue(result.Count >0);
                }
            }
        }


    }
}
