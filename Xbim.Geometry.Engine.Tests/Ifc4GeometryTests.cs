using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Ifc;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4.ProductExtension;
using Xbim.ModelGeometry.Scene;
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
                Assert.IsTrue((int)bar.Volume == 129879);
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
        [TestMethod]
        public void TriangulatedFaceSet2Test()
        {
            using (var model = IfcStore.Open(@"Ifc4TestFiles\beam-curved-i-shape-tessellated.ifc"))
            {
                var triangulatedFaceSet = model.Instances.OfType<IfcTriangulatedFaceSet>().FirstOrDefault();
                Assert.IsNotNull(triangulatedFaceSet);
                var geom = _xbimGeometryCreator.CreateSurfaceModel(triangulatedFaceSet);
                Assert.IsTrue(geom.Shells.Count == 1);
                Assert.IsTrue(Math.Abs(geom.Shells.First.BoundingBox.Volume - 13.337264) < 1e-5);

            }
        }

        [TestMethod]
        public void TriangulatedFaceSet3Test()
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
        public void TriangulatedFaceSet4Test()
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

        #endregion

    }
}
