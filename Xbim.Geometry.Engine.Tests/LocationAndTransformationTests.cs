using System;
using System.Text;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using Ifc4GeometryTests;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Step21;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricConstraintResource;
using Xbim.Ifc4.GeometryResource;

namespace GeometryTests
{
    /// <summary>
    /// Summary description for LocationAndTransformationTests
    /// </summary>
    [DeploymentItem(@"x64\", "x64")]
    [DeploymentItem(@"x86\", "x86")]
    [TestClass]
    public class LocationAndTransformationTests
    {
        XbimGeometryEngine _geomEngine;

        public LocationAndTransformationTests()
        {
            _geomEngine = new XbimGeometryEngine();
        }

        private TestContext testContextInstance;

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get { return testContextInstance; }
            set { testContextInstance = value; }
        }

        #region Additional test attributes

        //
        // You can use the following additional attributes as you write your tests:
        //
        // Use ClassInitialize to run code before running the first test in the class
        // [ClassInitialize()]
        // public static void MyClassInitialize(TestContext testContext) { }
        //
        // Use ClassCleanup to run code after all tests in a class have run
        // [ClassCleanup()]
        // public static void MyClassCleanup() { }
        //
        // Use TestInitialize to run code before running each test 
        // [TestInitialize()]
        // public void MyTestInitialize() { }
        //
        // Use TestCleanup to run code after each test has run
        // [TestCleanup()]
        // public void MyTestCleanup() { }
        //

        #endregion

        [TestMethod]
        public void MoveAndCopyTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (
                var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel)
                )
            {
                using (var txn = m.BeginTransaction())
                {
                    var block = IfcModelBuilder.MakeBlock(m, 10, 10, 10);
                    var solid = _geomEngine.CreateSolid(block);
                    var ax3D = IfcModelBuilder.MakeAxis2Placement3D(m);
                    ax3D.Location.Y = 100;
                    var solidA = _geomEngine.Moved(solid, ax3D) as IXbimSolid;
                    Assert.IsNotNull(solidA, "Should be the same type as the master");
                    Assert.IsTrue(Math.Abs(solidA.Volume - solid.Volume) < 1e-9, "Volume has changed");
                    var displacement = solidA.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    Assert.IsTrue(displacement == new XbimVector3D(0, 100, 0));
                    var bbA = solidA.BoundingBox;
                    var solidB = _geomEngine.Moved(solid, ax3D);
                    Assert.IsTrue(bbA.Centroid() - solidB.BoundingBox.Centroid() == new XbimVector3D(0, 0, 0));
                }
            }
        }

        [TestMethod]
        public void ScaleAndCopyTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (
                var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel)
                )
            {
                using (var txn = m.BeginTransaction())
                {
                    var block = IfcModelBuilder.MakeBlock(m, 10, 10, 10);
                    var solid = _geomEngine.CreateSolid(block);
                    var transform = IfcModelBuilder.MakeCartesianTransformationOperator3D(m);
                    transform.Scale = 2;
                    var solidA = _geomEngine.Transformed(solid, transform);
                    var bb = solid.BoundingBox;
                    var bbA = solidA.BoundingBox;
                    Assert.IsTrue(Math.Abs(bb.Volume - 1000) < 1e-9,"Bounding box volume is incorrect in original shape");
                    Assert.IsTrue(Math.Abs(bbA.Volume - 8000) < 1e-9, "Bounding box volume is incorrect in scaled shape");
                    var transformNonUniform = IfcModelBuilder.MakeCartesianTransformationOperator3DnonUniform(m);
                    transformNonUniform.Scale3 = 100;
                    var solidB = _geomEngine.Transformed(solid, transformNonUniform);
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
            using (
                var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel)
                )
            {
                using (var txn = m.BeginTransaction())
                {
                    var block = IfcModelBuilder.MakeBlock(m, 50, 10, 10);
                    var solid = _geomEngine.CreateSolid(block);
                    var placement = IfcModelBuilder.MakeLocalPlacement(m);
                    ((IfcAxis2Placement3D) placement.RelativePlacement).Location.X = 100;
                    var bb = solid.BoundingBox;
                    var solidA = _geomEngine.Moved(solid, placement) as IXbimSolid;
                    Assert.IsNotNull(solidA, "Should be the same type as the master");
                    var displacement = solidA.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    Assert.IsTrue(displacement == new XbimVector3D(100,0, 0));

                    var placementRelTo = ((IfcLocalPlacement) placement.PlacementRelTo);
                    var zDir = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 0, 1));
                    ((IfcAxis2Placement3D) placementRelTo.RelativePlacement).Axis = zDir;
                    var yDir = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, 1, 0));
                    ((IfcAxis2Placement3D)placementRelTo.RelativePlacement).RefDirection = yDir; //point in Y
                    ((IfcAxis2Placement3D)placementRelTo.RelativePlacement).Location.X = 2000;
                    var solidB = _geomEngine.Moved(solid, placement) as IXbimSolid;
                    displacement = solidB.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    var meshbuilder = new MeshHelper();
                    _geomEngine.Mesh(meshbuilder,solidB,m.ModelFactors.Precision,m.ModelFactors.DeflectionTolerance);
                    var box = meshbuilder.BoundingBox;
                    Assert.IsTrue(displacement == new XbimVector3D(1970,120,0));


                }
            }
        }

        [TestMethod]
        public void GridPlacementTest()
        {
            //this test checks that a object is correctly copied and moved
            //create a box
            using (
                var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel)
                )
            {
                using (var txn = m.BeginTransaction())
                {
                    var block = IfcModelBuilder.MakeBlock(m, 10, 10, 10);
                    var solid = _geomEngine.CreateSolid(block);
                    var grid = IfcModelBuilder.MakeGrid(m,3,100);
                    var gridPlacement = m.Instances.New <IfcGridPlacement>();
                    gridPlacement.PlacementLocation = m.Instances.New<IfcVirtualGridIntersection>();
                    gridPlacement.PlacementLocation.IntersectingAxes.Add(grid.UAxes.Last());
                    gridPlacement.PlacementLocation.IntersectingAxes.Add(grid.VAxes.Last());
                    var solidA = _geomEngine.Moved(solid, gridPlacement) as IXbimSolid;
                    Assert.IsNotNull(solidA, "Should be the same type as the master");
                    var displacement = solidA.BoundingBox.Centroid() - solid.BoundingBox.Centroid();
                    Assert.IsTrue(displacement == new XbimVector3D(200, 200, 0));
                }
            }
        }
    }
}
    
    
