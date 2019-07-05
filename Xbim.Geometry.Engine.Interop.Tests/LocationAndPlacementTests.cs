using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Ifc4.GeometricConstraintResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class LocationAndPlacementTests
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
        public void MoveAndCopyTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    {
                        var block = IfcModelBuilder.MakeBlock(m, 10, 10, 10);
                        var solid = geomEngine.CreateSolid(block);
                        var ax3D = IfcModelBuilder.MakeAxis2Placement3D(m);
                        ax3D.Location.Y = 100;
                        var solidA = geomEngine.Moved(solid, ax3D) as IXbimSolid;
                        Assert.IsNotNull(solidA, "Should be the same type as the master");
                        Assert.IsTrue(Math.Abs(solidA.Volume - solid.Volume) < 1e-9, "Volume has changed");
                        var displacement = solidA.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                        Assert.IsTrue(displacement == new XbimVector3D(0, 100, 0));
                        var bbA = solidA.BoundingBox;
                        var solidB = geomEngine.Moved(solid, ax3D);
                        Assert.IsTrue(bbA.Centroid() - solidB.BoundingBox.Centroid() == new XbimVector3D(0, 0, 0));
                    }
                }
            }
        }

        [TestMethod]
        public void ScaleAndCopyTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var block = IfcModelBuilder.MakeBlock(m, 10, 10, 10);
                    var solid = geomEngine.CreateSolid(block);
                    var transform = IfcModelBuilder.MakeCartesianTransformationOperator3D(m);
                    transform.Scale = 2;
                    var solidA = geomEngine.Transformed(solid, transform);
                    var bb = solid.BoundingBox;
                    var bbA = solidA.BoundingBox;
                    Assert.IsTrue(Math.Abs(bb.Volume - 1000) < 1e-9, "Bounding box volume is incorrect in original shape");
                    Assert.IsTrue(Math.Abs(bbA.Volume - 8000) < 1e-9, "Bounding box volume is incorrect in scaled shape");
                    var transformNonUniform = IfcModelBuilder.MakeCartesianTransformationOperator3DnonUniform(m);
                    transformNonUniform.Scale3 = 100;
                    var solidB = geomEngine.Transformed(solid, transformNonUniform);
                    Assert.IsTrue(Math.Abs(solidB.BoundingBox.Volume - 100000) < 1e-9, "Bounding box volume is incorrect in non uniform scaled shape");
                    Assert.IsTrue(Math.Abs(bb.Volume - 1000) < 1e-9, "Bounding box volume of original shape as been changed");
                }
            }
        }

        [TestMethod]
        public void ObjectPlacementTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var block = IfcModelBuilder.MakeBlock(m, 50, 10, 10);
                    var solid = geomEngine.CreateSolid(block);
                    var placement = IfcModelBuilder.MakeLocalPlacement(m);
                    ((IfcAxis2Placement3D)placement.RelativePlacement).Location.X = 100;
                    var bb = solid.BoundingBox;
                    var solidA = geomEngine.Moved(solid, placement) as IXbimSolid;
                    Assert.IsNotNull(solidA, "Should be the same type as the master");
                    var displacement = solidA.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    Assert.IsTrue(displacement == new XbimVector3D(100, 0, 0));

                    var placementRelTo = ((IfcLocalPlacement)placement.PlacementRelTo);
                    var zDir = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 0, 1));
                    ((IfcAxis2Placement3D)placementRelTo.RelativePlacement).Axis = zDir;
                    var yDir = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 1, 0));
                    ((IfcAxis2Placement3D)placementRelTo.RelativePlacement).RefDirection = yDir; //point in Y
                    ((IfcAxis2Placement3D)placementRelTo.RelativePlacement).Location.X = 2000;
                    var solidB = geomEngine.Moved(solid, placement) as IXbimSolid;
                    displacement = solidB.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    var meshbuilder = new MeshHelper();
                    geomEngine.Mesh(meshbuilder, solidB, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var box = meshbuilder.BoundingBox;
                    Assert.IsTrue(displacement == new XbimVector3D(1970, 120, 0));


                }
            }
        }

        [TestMethod]
        public void GridPlacementTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var block = IfcModelBuilder.MakeBlock(m, 10, 10, 10);
                    var solid = geomEngine.CreateSolid(block);
                    var grid = IfcModelBuilder.MakeGrid(m, 3, 100);
                    var gridPlacement = m.Instances.New<IfcGridPlacement>();
                    gridPlacement.PlacementLocation = m.Instances.New<IfcVirtualGridIntersection>();
                    gridPlacement.PlacementLocation.IntersectingAxes.Add(grid.UAxes.Last());
                    gridPlacement.PlacementLocation.IntersectingAxes.Add(grid.VAxes.Last());
                    var solidA = geomEngine.Moved(solid, gridPlacement) as IXbimSolid;
                    Assert.IsNotNull(solidA, "Should be the same type as the master");
                    var displacement = solidA.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    Assert.IsTrue(displacement == new XbimVector3D(200, 200, 0));
                }
            }
        }
        [TestMethod]
        public void TransformSolidRectangularProfileDef()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {

                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = geomEngine.CreateSolid(extrude);
                    var transform = new XbimMatrix3D(); //test first with identity
                    var solid2 = (IXbimSolid)solid.Transform(transform);
                    var s1Verts = solid.Vertices.ToList();
                    var s2Verts = solid2.Vertices.ToList();
                    for (int i = 0; i < s1Verts.Count; i++)
                    {
                        XbimVector3D v = s1Verts[i].VertexGeometry - s2Verts[i].VertexGeometry;
                        Assert.IsTrue(v.Length < m.ModelFactors.Precision, "vertices not the same");
                    }
                    transform.RotateAroundXAxis(Math.PI / 2);
                    transform.RotateAroundYAxis(Math.PI / 4);
                    transform.RotateAroundZAxis(Math.PI);
                    transform.OffsetX += 100;
                    transform.OffsetY += 200;
                    transform.OffsetZ += 300;
                    solid2 = (IXbimSolid)solid.Transform(transform);
                    Assert.IsTrue(Math.Abs(solid.Volume - solid2.Volume) < 0.001, "Volume differs");
                    transform.Invert();
                    solid2 = (IXbimSolid)solid2.Transform(transform);
                    s1Verts = solid.Vertices.ToList();
                    s2Verts = solid2.Vertices.ToList();
                    for (int i = 0; i < s1Verts.Count; i++)
                    {
                        XbimVector3D v = s1Verts[i].VertexGeometry - s2Verts[i].VertexGeometry;
                        Assert.IsTrue(v.Length < m.ModelFactors.Precision, "vertices not the same");
                    }
                    txn.Commit();
                }
            }
        }
    }
}
