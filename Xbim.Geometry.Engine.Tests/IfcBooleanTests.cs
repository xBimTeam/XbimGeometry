using System;
using System.IO;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc2x3.ProfileResource;
using Xbim.Ifc2x3.TopologyResource;
using Xbim.IO;
using Xbim.Common.Logging;
using XbimGeometry.Interfaces;

namespace GeometryTests
{
    [TestClass]
    public class IfcBooleanTests
    {
        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void BooleanCutShellTest()
        {
            using (var m = new XbimModel())
            {
                try
                {
                    m.CreateFrom("SolidTestFiles\\7- Boolean_IfcHalfspace_With_IfcExtrudedAreaSolid.ifc", null, null,
                        true, true);
                    var fbsm = m.Instances[57] as IfcConnectedFaceSet;
                    Assert.IsFalse(fbsm == null, "IfcConnectedFaceSet is invalid");

                    var shell = _xbimGeometryCreator.CreateShell(fbsm);
                    
                    using (var txn = m.BeginTransaction())
                    {
                        var ifcCylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 500, 1500);
                        ifcCylinder.Position.Location.SetXYZ(-23000, 12000, 2000);
                        var cylinder = _xbimGeometryCreator.CreateSolid(ifcCylinder);
                        var shellSet = (IXbimShellSet) shell.Cut(cylinder, m.ModelFactors.PrecisionBoolean);
                        Assert.IsTrue(shellSet.Count == 1, "Cutting this shell should return a single shell");
                        var resultShell = shellSet.First;
                        Assert.IsTrue(shell.SurfaceArea > resultShell.SurfaceArea,
                            "The surface area of the result should be less than the original");
                    }
                }
                finally
                {
                    var path = m.DatabaseName;
                    m.Close();
                    File.Delete(path);
                }

            }
        }

        [TestMethod]
        public void BooleanIntersectonShellTest()
        {
            using (var m = new XbimModel())
            {
                try
                {


                    m.CreateFrom("SolidTestFiles\\7- Boolean_IfcHalfspace_With_IfcExtrudedAreaSolid.ifc", null, null, true, true);
                    var fbsm1 = m.Instances[57] as IfcConnectedFaceSet;
                    Assert.IsNotNull(fbsm1, "IfcConnectedFaceSet is invalid");
                    using (var txn = m.BeginTransaction())
                    {


                        var ifcCylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 500, 1500);
                        ifcCylinder.Position.Location.SetXYZ(-23000, 12000, 2000);
                        var cylinder = _xbimGeometryCreator.CreateSolid(ifcCylinder);
                        var shell1 = _xbimGeometryCreator.CreateShell(fbsm1);
                        var shell2 = ((IXbimShellSet)shell1.Cut(cylinder, m.ModelFactors.PrecisionBoolean)).First;
                        var shellSet = (IXbimShellSet)shell1.Intersection(shell2, m.ModelFactors.PrecisionBoolean);
                        Assert.IsTrue(shellSet.Count == 1,
                            string.Format("Cutting this shell should return a single shell, it returned {0}", shellSet.Count));
                        var resultShell = shellSet.First;
                        Assert.IsTrue(Math.Abs(shell2.SurfaceArea - resultShell.SurfaceArea) < m.ModelFactors.Precision,
                            "The surface area of the result should be the same as the second shell");

                    }
                }
                finally
                {
                    var path = m.DatabaseName;
                    m.Close();
                    File.Delete(path);
                }
            }
        }

        [TestMethod]
        public void BooleanUnionShellTest()
        {
            using (var m = new XbimModel())
            {
                try
                {

                    m.CreateFrom("SolidTestFiles\\7- Boolean_IfcHalfspace_With_IfcExtrudedAreaSolid.ifc", null, null, true, true);
                    var fbsm1 = m.Instances[57] as IfcConnectedFaceSet;
                    Assert.IsNotNull(fbsm1, "IfcConnectedFaceSet is invalid");
                    var fbsm2 = m.Instances[305] as IfcConnectedFaceSet;
                    Assert.IsNotNull(fbsm2, "IfcConnectedFaceSet is invalid");

                    var shell1 = _xbimGeometryCreator.CreateShell(fbsm1);
                    var shell2 = _xbimGeometryCreator.CreateShell(fbsm2);
                    var shellSet = (IXbimShellSet)shell1.Union(shell2, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(shellSet.Count == 1,
                        string.Format("Cutting this shell should return a single shell, it returned {0}", shellSet.Count));
                    var resultShell = shellSet.First;
                    Assert.IsTrue(shell1.SurfaceArea < resultShell.SurfaceArea,
                        "The surface area of the result should be less than the original");
                }
                finally
                {
                    var path = m.DatabaseName;
                    m.Close();
                    File.Delete(path);
                }
            }
        }
        [TestMethod]
        public void BooleanCutSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    
                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    
                    var outer = _xbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = _xbimGeometryCreator.CreateSolid(cylinderInner);
                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet) result;
                    Assert.IsTrue(solidSet.Count ==1 , "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);
                   
                }
            }
        }

        [TestMethod]
        public void BooleanUnionSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    
                    var a = _xbimGeometryCreator.CreateSolid(sphere);
                    var b = _xbimGeometryCreator.CreateSolid(cylinder);
                    var result = a.Union(b, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet) result;
                    Assert.IsTrue(solidSet.Count ==1 , "unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);
                   
                }
            }
        }

        [TestMethod]
        public void BooleanIntersectSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    
                    var a = _xbimGeometryCreator.CreateSolid(sphere);
                    var b = _xbimGeometryCreator.CreateSolid(cylinder);
                    var result = a.Intersection(b, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "intersecting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }


        [TestMethod]
        public void SectionOfCylinderTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    
                    var solid = _xbimGeometryCreator.CreateSolid(cylinder);
                    IfcPlane plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinder.Position.Location.X + 1, cylinder.Position.Location.Y, cylinder.Position.Location.Z), new XbimVector3D(0, -1, 0), new XbimVector3D(1, 0, 0));
                    var cutPlane = _xbimGeometryCreator.CreateFace(plane);
                    var section = solid.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(section.First != null, "Result should be a face");
                    Assert.IsTrue(section.First.OuterBound.Edges.Count == 4, "4 edges are required for this section of a cylinder");
                    //repeat with section through cylinder
                    plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinder.Position.Location.X + 1, cylinder.Position.Location.Y, cylinder.Position.Location.Z), new XbimVector3D(0, 0, 1), new XbimVector3D(0, 1, 0));
                    cutPlane = _xbimGeometryCreator.CreateFace(plane);
                    section = solid.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(section.First != null, "Result should be a face");
                    Assert.IsTrue(section.First.OuterBound.Edges.Count == 1, "1 edge is required for this section of a cylinder");
                    Assert.IsTrue(section.First.InnerBounds.Count == 0, "0 inner wires are required for this section of a cylinder");
                }
            }
        }

        [TestMethod]
        public void SectionWithInnerWireTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();
                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    bresult.FirstOperand = cylinderOuter;
                    bresult.SecondOperand = cylinderInner;
                    bresult.Operator = IfcBooleanOperator.Difference;
                    csgTree.TreeRootExpression = bresult;
                    
                    var solid = _xbimGeometryCreator.CreateSolid(csgTree);
                    IfcPlane plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinderInner.Position.Location.X + 1, cylinderInner.Position.Location.Y, cylinderInner.Position.Location.Z), new XbimVector3D(0, 0, 1), new XbimVector3D(0, 1, 0));
                    var cutPlane = _xbimGeometryCreator.CreateFace(plane);
                    var section = solid.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(section.First != null, "Result should be a face");
                    Assert.IsTrue(section.First.OuterBound.Edges.Count == 1, "1 edge is required for this section of a cylinder");
                    Assert.IsTrue(section.First.InnerBounds.Count == 1, "1 inner wire is required for this section of a cylinder");
                }
            }
        }

        [TestMethod]
        public void SectionOfBlockTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);

                    
                    var solid = _xbimGeometryCreator.CreateSolid(block);
                    IfcPlane plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(block.Position.Location.X + 5, block.Position.Location.Y, block.Position.Location.Z), new XbimVector3D(-1, 0, 0), new XbimVector3D(0, 1, 0));
                    var cutPlane = _xbimGeometryCreator.CreateFace(plane);

                    var section = solid.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    if (section.First == null)
                    {
                        Assert.IsTrue(section.First != null, "Result should be a single face");
                        Assert.IsTrue(section.First.OuterBound.Edges.Count == 4, "4 edges are required of a section of a block");
                    }
                }
            }
        }


        [TestMethod]
        public void IfcCsgDifferenceTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();
                    
                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    bresult.FirstOperand = cylinderOuter;
                    bresult.SecondOperand = cylinderInner;
                    bresult.Operator = IfcBooleanOperator.Difference;
                    csgTree.TreeRootExpression = bresult;
                    
                    var solid = _xbimGeometryCreator.CreateSolid(csgTree);
                    Assert.IsTrue(solid.Faces.Count == 4, "4 faces are required of this csg solid");
                    Assert.IsTrue(solid.Vertices.Count == 4, "4 vertices are required of this csg solid");
                }
            }
        }

        [TestMethod]
        public void IfcCsgUnionTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();
                    
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    bresult.FirstOperand = cylinder;
                    bresult.SecondOperand = sphere;
                    bresult.Operator = IfcBooleanOperator.Union;
                    csgTree.TreeRootExpression = bresult;
                    
                    var solid = _xbimGeometryCreator.CreateSolid(csgTree);
                    Assert.IsTrue(solid.Faces.Count == 3, "3 faces are required of this csg solid");
                    Assert.IsTrue(solid.Vertices.Count == 3, "3 vertices are required of this csg solid");

                }
            }
        }

        [TestMethod]
        public void IfcCsgIntersectionTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();
                    
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    bresult.FirstOperand = cylinder;
                    bresult.SecondOperand = sphere;
                    bresult.Operator = IfcBooleanOperator.Intersection;
                    csgTree.TreeRootExpression = bresult;
                    
                    var solid = _xbimGeometryCreator.CreateSolid(csgTree);
                    Assert.IsTrue(solid.Faces.Count == 3, "3 faces are required of this csg solid");
                    Assert.IsTrue(solid.Vertices.Count == 3, "3 vertices are required of this csg solid");
                }
            }
        }



        [TestMethod]
        public void IfcHalfspace_With_IfcExtrudedAreaSolid()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\7- Boolean_IfcHalfspace_With_IfcExtrudedAreaSolid.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcBooleanClippingResult>().FirstOrDefault(hs => hs.FirstOperand is IfcExtrudedAreaSolid && hs.SecondOperand is IfcHalfSpaceSolid);
                    Assert.IsTrue(eas != null, "No IfcBooleanClippingResult found");
                    Assert.IsTrue(eas.FirstOperand is IfcExtrudedAreaSolid , "Incorrect first operand found");
                    Assert.IsTrue(((IfcExtrudedAreaSolid)eas.FirstOperand).SweptArea is IfcRectangleProfileDef, "Incorrect profile definition for extrusion found");
                    Assert.IsTrue(eas.SecondOperand is IfcHalfSpaceSolid, "Incorrect second operand found");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call

                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");

                }
            }
        }
        
        [TestMethod]
        public void IfcPolygonalBoundedHalfspace_With_IfcExtrudedAreaSolid()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\8- Boolean_IfcPolygonalBoundedHalfspace_With_IfcExtrudedAreaSolid.ifc", null, null, true, true);
                    var eas = m.Instances.OfType<IfcBooleanClippingResult>().FirstOrDefault(hs => hs.FirstOperand is IfcExtrudedAreaSolid && hs.SecondOperand.GetType() == typeof(IfcPolygonalBoundedHalfSpace));
                    Assert.IsTrue(eas != null, "No IfcBooleanClippingResult found");
                    Assert.IsTrue(eas.FirstOperand is IfcExtrudedAreaSolid , "Incorrect first operand found");
                    Assert.IsTrue(((IfcExtrudedAreaSolid)eas.FirstOperand).SweptArea is IfcRectangleProfileDef, "Incorrect profile definition for extrusion found");
                    Assert.IsTrue(eas.SecondOperand is IfcHalfSpaceSolid, "Incorrect second operand found");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 6, "This solid should have 6 faces");

                }
            }
        }


        [TestMethod]
        public void Nested_Boolean_Operations()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\9- Boolean_With_Nested_Booleans.ifc", null, null, true, true);
                    var eas = m.Instances[32] as IfcBooleanClippingResult;
                    Assert.IsTrue(eas != null, "No IfcBooleanClippingResult found");
                    Assert.IsTrue(eas.FirstOperand is IfcBooleanClippingResult, "Incorrect first operand found");
                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                    IfcCsgTests.GeneralTest(solid);
                    Assert.IsTrue(solid.Faces.Count() == 6, "This solid should have 6 faces");

                }
            }
        }

        [TestMethod]
        public void Boolean_With_BoxedHalfSpace()
        {
            using (var eventTrace = LoggerFactory.CreateEventTrace())
            {
                using (var m = new XbimModel())
                {
                    m.CreateFrom("SolidTestFiles\\10- Boxed Half Space.ifc", null, null, true, true);
                    var eas = m.Instances[28] as IfcBooleanClippingResult;
                    Assert.IsTrue(eas != null, "No IfcBooleanClippingResult found");
                    Assert.IsTrue(eas.SecondOperand is IfcBoxedHalfSpace, "Incorrect second operand found");
                                    
                    var solid = _xbimGeometryCreator.CreateSolid(eas);
                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
                    IfcCsgTests.GeneralTest(solid);
                  //  var w = new XbimOccWriter();
                  //  w.Write(solid, "d:\\xbim\\s");
                    Assert.IsTrue(solid.Faces.Count() == 8, "This solid should have 8 faces");
                }
            }
        }
 #if USE_CARVE_CSG
        
 #region Mixed cut tests

		[TestMethod]
        public void BooleanCutSolidWithFacetedSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanCutSolidWithFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);

                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Cut(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanCutFacetedSolidWithFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Cut(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanCutFacetedSolidWithFacetedSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanCutFacetedSolidWithSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Cut(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanCutFacetedSolidWithSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsFalse(solidSet.First.IsPolyhedron, "This should return a shape with curves");
                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        } 
        #endregion

  


        #region Mixed union tests

        [TestMethod]
        public void BooleanUnionSolidWithFacetedSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Union(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);
                }
            }
        }

        [TestMethod]
        public void BooleanUnionSolidWithFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);

                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Union(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanUnionFacetedSolidWithFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Union(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanUnionFacetedSolidWithFacetedSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Union(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanUnionFacetedSolidWithSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Union(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanUnionFacetedSolidWithSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Union(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsFalse(solidSet.First.IsPolyhedron, "This should return a shape with curves");
                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }
        #endregion

        #region Mixed intersection tests

        [TestMethod]
        public void BooleanIntersectSolidWithFacetedSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Intersection(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);
                }
            }
        }

        [TestMethod]
        public void BooleanIntersectSolidWithFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);

                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Intersection(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanIntersectFacetedSolidWithFacetedSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Intersection(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanIntersectFacetedSolidWithFacetedSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Intersection(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanIntersectFacetedSolidWithSolidPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small = XbimGeometryCreator.CreateSolid(block2);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = big.Intersection(small, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

        [TestMethod]
        public void BooleanIntersectFacetedSolidWithSolidNonPlanarTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
                        m.ModelFactors.DeflectionTolerance);
                    var result = outer.Intersection(inner, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsFalse(solidSet.First.IsPolyhedron, "This should return a shape with curves");
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }
        #endregion

#endif
        #region Compound booleans

        [TestMethod]
        public void BooleanCutMultipleSolidsFromASinglePlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
                    block3.Position.Location.X += 3;
                    block3.Position.Location.Y += 3;
                    var big = _xbimGeometryCreator.CreateSolid(block1);
                    var small1 = _xbimGeometryCreator.CreateSolid(block2);
                    var small2 = _xbimGeometryCreator.CreateSolid(block3);
                    var solids = _xbimGeometryCreator.CreateSolidSet();
                    solids.Add(small1);
                    solids.Add(small2);
                    var result = big.Cut(solids, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

#if USE_CARVE_CSG
        [TestMethod]
        public void BooleanCutMultipleSolidsFromASingleFacetedPlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
                    block3.Position.Location.X += 3;
                    block3.Position.Location.Y += 3;
                    var big = XbimGeometryCreator.CreateSolid(block1);
                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var small1 = XbimGeometryCreator.CreateSolid(block2);
                    var small2 = XbimGeometryCreator.CreateSolid(block3);
                    var solids = XbimGeometryCreator.CreateSolidSet();
                    solids.Add(small1);
                    solids.Add(small2);
                    var result = big.Cut(solids, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }
        
#endif
        [TestMethod]
        public void BooleanCutMultipleSolidsFromASingleNonPlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 15);
                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
                    block2.Position.Location.X += 6;
                    block2.Position.Location.Y += 6;
                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
                    block3.Position.Location.X += 3;
                    block3.Position.Location.Y += 3;
                    var big = _xbimGeometryCreator.CreateSolid(block1);
                    var small1 = _xbimGeometryCreator.CreateSolid(block2);
                    var small2 = _xbimGeometryCreator.CreateSolid(block3);
                    var solids = _xbimGeometryCreator.CreateSolidSet();
                    solids.Add(small1);
                    solids.Add(small2);
                    var result = big.Cut(solids, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        }

#if USE_CARVE_CSG
        [TestMethod]
        public void BooleanCutMultipleFacetedSolidsFromASingleNonPlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 15);
                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
                    block2.Position.Location.X += 6;
                    block2.Position.Location.Y += 6;
                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
                    block3.Position.Location.X += 3;
                    block3.Position.Location.Y += 3;
                    var big = XbimGeometryCreator.CreateSolid(block1);
                    var small1 = XbimGeometryCreator.CreateSolid(block2);
                    var small2 = XbimGeometryCreator.CreateSolid(block3);
                    small1 = XbimGeometryCreator.CreateFacetedSolid(small1, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    small2 = XbimGeometryCreator.CreateFacetedSolid(small2, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
                    var solids = XbimGeometryCreator.CreateSolidSet();
                    solids.Add(small1);
                    solids.Add(small2);
                    var result = big.Cut(solids, m.ModelFactors.PrecisionBoolean);
                    var solidSet = (IXbimSolidSet)result;
                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
                    IfcCsgTests.GeneralTest(solidSet.First);

                }
            }
        } 
#endif

        #endregion
        #region Single solid from multiple solids

        [TestMethod]
        public void BooleanCutSingleSolidsFromAMultipleNonPlanarSolidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
                    block2.Position.Location.Z += 22; //stack block2 2 above block1
                    var block3 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    block3.Position.Location.Z += 10; //in bretween 1 and 2
                    var b1 = _xbimGeometryCreator.CreateSolid(block1);
                    var b2 = _xbimGeometryCreator.CreateSolid(block2);
                    var b3 = _xbimGeometryCreator.CreateSolid(block3);
                    var solids = _xbimGeometryCreator.CreateSolidSet();
                    solids.Add(b1);
                    solids.Add(b2);
                    var result = solids.Cut(b3, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(result.Count == 2, "Cutting of these two solids should return two  solids");
                    
                    foreach (var solid in result)
                         IfcCsgTests.GeneralTest(solid);
                }
            }
        }

        #if USE_CARVE_CSG
        [TestMethod]
        public void BooleanCutSingleFacetedSolidsFromMultipleNonPlanarSolidsTest()
        {
           using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
                    block2.Position.Location.Z += 22; //stack block2 2 above block1
                    var block3 = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    block3.Position.Location.Z += 10; //in bretween 1 and 2
                    var b1 = XbimGeometryCreator.CreateSolid(block1);
                    var b2 = XbimGeometryCreator.CreateSolid(block2);
                    var b3 = XbimGeometryCreator.CreateSolid(block3);
                    var solids = XbimGeometryCreator.CreateSolidSet();
                    solids.Add(b1);
                    solids.Add(b2);
                    var result = solids.Cut(b3, m.ModelFactors.PrecisionBoolean);

                    Assert.IsTrue(result.Count == 2, "Cutting of these two solids should return two  solids");
                   
                    foreach (var solid in result)
                         IfcCsgTests.GeneralTest(solid);
                    
                }
            }
        } 
#endif
        #endregion

        #region Solid with voids test
         [TestMethod]
        public void BooleanCutSolidWithVoidPlanarTest()
        {
           using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 20, 20, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 5, 5);
                    block2.Position.Location.X += 10;
                    block2.Position.Location.Y += 10; 
                    block2.Position.Location.Z += 10; 
                   
                    var b1 = _xbimGeometryCreator.CreateSolid(block1);
                    var b2 = _xbimGeometryCreator.CreateSolid(block2);
                    var result = b1.Cut(b2, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(result.Count == 1, "Cutting of these two solids should return two  solids");                  
                    foreach (var solid in result)
                         IfcCsgTests.GeneralTest(solid);
                    
                }
            }
        }

         [TestMethod]
         public void BooleanCutSolidWithVoidNonPlanarTest()
         {
             using (var m = XbimModel.CreateTemporaryModel())
             {
                 using (var txn = m.BeginTransaction())
                 {

                     var block1 = IfcModelBuilder.MakeSphere(m, 20);
                     var block2 = IfcModelBuilder.MakeSphere(m, 5);
            
                     var b1 = _xbimGeometryCreator.CreateSolid(block1);
                     var b2 = _xbimGeometryCreator.CreateSolid(block2);
                     var result = b1.Cut(b2, m.ModelFactors.PrecisionBoolean);
                     Assert.IsTrue(result.Count == 1, "Cutting of these two solids should return two  solids");
                     const double vOuter = (4.0 / 3.0)* Math.PI * 20.0 * 20.0 * 20.0;
                     const double vInner = (4.0 / 3.0) * Math.PI * 5.0 * 5.0 * 5.0;
                     const double volume = vOuter - vInner;

                     Assert.IsTrue(result.First.Volume - volume <= m.ModelFactors.Precision,"Volume is incorrect");

                 }
             }
         } 
        #endregion
    }
}
