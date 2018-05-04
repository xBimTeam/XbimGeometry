using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Ifc4.Interfaces;
using Microsoft.Extensions.Logging;
using System.Linq;
using Xbim.IO.Memory;
using Xbim.Common.Geometry;
using System.Diagnostics;
using System;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;

namespace Xbim.Geometry.Engine.Interop.Tests
{


    [TestClass]
    public class IfcBooleanTests
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<IfcBooleanTests>();
        }

        public void IsSolidTest(IXbimSolid solid, bool ignoreVolume = false, bool isHalfSpace = false, int entityLabel = 0)
        {
            // ReSharper disable once CompareOfFloatsByEqualityOperator
            if (ignoreVolume && !isHalfSpace && solid.Volume == 0)
            {
                Trace.WriteLine(String.Format("Entity  #{0} has zero volume>", entityLabel));
            }
            if (!ignoreVolume) Assert.IsTrue(solid.Volume > 0, "Volume should be greater than 0");
            Assert.IsTrue(solid.SurfaceArea > 0, "Surface Area should be greater than 0");
            Assert.IsTrue(solid.IsValid);

            if (!isHalfSpace)
            {
                foreach (var face in solid.Faces)
                {

                    Assert.IsTrue(face.OuterBound.IsValid, "Face has no outer bound in #" + entityLabel);

                    Assert.IsTrue(face.Area > 0, "Face area should be greater than 0 in #" + entityLabel);
                    Assert.IsTrue(face.Perimeter > 0, "Face perimeter should be breater than 0 in #" + entityLabel);

                    if (face.IsPlanar)
                    {
                        Assert.IsTrue(!face.Normal.IsInvalid(), "Face normal is invalid in #" + entityLabel);
                        //  Assert.IsTrue(face.OuterBound.Edges.Count>2, "A face should have at least 3 edges");
                        //   Assert.IsTrue(!face.OuterBound.Normal.IsInvalid(), "Face outerbound normal is invalid in #" + entityLabel);
                        //   Assert.IsTrue(face.OuterBound.IsPlanar, "Face is planar but wire is not in #" + entityLabel);
                    }
                    else
                        Assert.IsFalse(face.OuterBound.IsPlanar, "Face is not planar but wire is planar in #" + entityLabel);
                    foreach (var edge in face.OuterBound.Edges)
                    {
                        Assert.IsTrue(edge.EdgeGeometry.IsValid, "Edge element is invalid in #" + entityLabel);
                        Assert.IsTrue(edge.EdgeStart.IsValid, "Edge start is invalid in #" + entityLabel);
                        Assert.IsTrue(edge.EdgeEnd.IsValid, "Edge end is invalid in #" + entityLabel);
                    }
                }
            }
        }

        /// <summary>
        /// This test subtracts a cuboid formed from a closed shell from a cuboid formed from and extruded area solid, the two are identical
        /// </summary>
        [TestMethod]
        public void EmptyBooleanResultTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(EmptyBooleanResultTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsFalse(solid.Faces.Any(), "This solid should have 0 faces");
            }

        }
        /// <summary>
        /// Just cuts two extruded area solids from each other to leave a cuboid
        /// </summary>
        [TestMethod]
        public void SimpleBooleanClipResultTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(SimpleBooleanClipResultTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcBooleanClippingResult found");
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsTrue(solid.Faces.Count==6, "This solid should have 6 faces");
            }

        }

        //[TestMethod]
        //public void CompositeProfileWithCutsTimeoutsTest()
        //{
        //    var path = Path.GetFullPath($@"TestFiles\{nameof(CompositeProfileWithCutsTimeoutsTest)}.proto");
        //    using (var input = File.Open(path,FileMode.Open))
        //    {
        //        var shapeGeometryDtoCopy = new ShapeGeometryDTO();
        //        shapeGeometryDtoCopy.MergeDelimitedFrom(input);
        //        var time = HelperFunctions.ConvertGeometryAllCompositesAtOnce(geomEngine, shapeGeometryDtoCopy,logger);
        //        Assert.IsTrue(time < 60000);//the default engine timeout
        //        Assert.IsTrue(time < 5000);//this is what we expect
        //    }
        //}
        
        /// <summary>
        /// Cuts one cylinder from another and returns a valid solid
        /// </summary>
        [TestMethod]
        public void CutTwoCylindersTest()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    var logger = loggerFactory.CreateLogger<IfcBooleanTests>();
                    var outer = geomEngine.CreateSolid(cylinderOuter, logger);
                    var inner = geomEngine.CreateSolid(cylinderInner, logger);
                    var solidSet = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);

                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
                    IsSolidTest(solidSet.First);
                    txn.Commit();
                }
            }
        }

        /// <summary>
        /// Unions a sphere and a cylinder
        /// </summary>
        [TestMethod]
        public void BooleanUnionSolidTest()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);

                    var a = geomEngine.CreateSolid(sphere, logger);
                    var b = geomEngine.CreateSolid(cylinder, logger);
                    var solidSet = a.Union(b, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(solidSet.Count == 1, "unioning these two solids should return a single solid");
                    IsSolidTest(solidSet.First);                   
                }
            }
        }

        /// <summary>
        /// Intersects a cylinder and a sphere
        /// </summary>
        [TestMethod]
        public void BooleanIntersectSolidTest()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    var a = geomEngine.CreateSolid(sphere, logger);
                    var b = geomEngine.CreateSolid(cylinder, logger);
                    var solidSet = a.Intersection(b, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(solidSet.Count == 1, "intersecting these two solids should return a single solid");
                    IsSolidTest(solidSet.First);
                }
            }
        }


        [TestMethod]
        public void SectionOfCylinderTest()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    var solid = geomEngine.CreateSolid(cylinder, logger);
                    var plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinder.Position.Location.X + 1, cylinder.Position.Location.Y, cylinder.Position.Location.Z), new XbimVector3D(0, -1, 0), new XbimVector3D(1, 0, 0));
                    var cutPlane = geomEngine.CreateFace(plane, logger);
                    var section = solid.Section(cutPlane, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(section.First != null, "Result should be a face");
                    Assert.IsTrue(section.First.OuterBound.Edges.Count == 4, "4 edges are required for this section of a cylinder");
                    //repeat with section through cylinder
                    plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinder.Position.Location.X + 1, cylinder.Position.Location.Y, cylinder.Position.Location.Z), new XbimVector3D(0, 0, 1), new XbimVector3D(0, 1, 0));
                    cutPlane = geomEngine.CreateFace(plane, logger);
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
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {

                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();
                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    bresult.FirstOperand = cylinderOuter;
                    bresult.SecondOperand = cylinderInner;
                    bresult.Operator = IfcBooleanOperator.DIFFERENCE;
                    csgTree.TreeRootExpression = bresult;

                    var solid = geomEngine.CreateSolid(csgTree,logger);
                    var plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(cylinderInner.Position.Location.X + 1, cylinderInner.Position.Location.Y, cylinderInner.Position.Location.Z), new XbimVector3D(0, 0, 1), new XbimVector3D(0, 1, 0));
                    var cutPlane = geomEngine.CreateFace(plane,logger);
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
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    var solid = geomEngine.CreateSolid(block, logger);
                    var plane = IfcModelBuilder.MakePlane(m, new XbimPoint3D(block.Position.Location.X + 5, block.Position.Location.Y, block.Position.Location.Z), new XbimVector3D(-1, 0, 0), new XbimVector3D(0, 1, 0));
                    var cutPlane = geomEngine.CreateFace(plane, logger);

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
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();

                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);
                    bresult.FirstOperand = cylinderOuter;
                    bresult.SecondOperand = cylinderInner;
                    bresult.Operator = IfcBooleanOperator.DIFFERENCE;
                    csgTree.TreeRootExpression = bresult;

                    var solid = geomEngine.CreateSolid(csgTree,logger);
                    Assert.IsTrue(solid.Faces.Count == 4, "4 faces are required of this csg solid");
                    Assert.IsTrue(solid.Vertices.Count == 4, "4 vertices are required of this csg solid");
                   
                }
            }
        }

        [TestMethod]
        public void IfcCsgUnionTest()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();

                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    bresult.FirstOperand = cylinder;
                    bresult.SecondOperand = sphere;
                    bresult.Operator = IfcBooleanOperator.UNION;
                    csgTree.TreeRootExpression = bresult;

                    var solid = geomEngine.CreateSolid(csgTree,logger);
                    Assert.IsTrue(solid.Faces.Count == 3, "3 faces are required of this csg solid");
                    Assert.IsTrue(solid.Vertices.Count == 3, "3 vertices are required of this csg solid");
                    
                }
            }
        }

        [TestMethod]
        public void IfcCsgIntersectionTest()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var csgTree = m.Instances.New<IfcCsgSolid>();
                    var bresult = m.Instances.New<IfcBooleanResult>();

                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
                    var sphere = IfcModelBuilder.MakeSphere(m, 15);
                    bresult.FirstOperand = cylinder;
                    bresult.SecondOperand = sphere;
                    bresult.Operator = IfcBooleanOperator.INTERSECTION;
                    csgTree.TreeRootExpression = bresult;

                    var solid = geomEngine.CreateSolid(csgTree, logger);
                    Assert.IsTrue(solid.Faces.Count == 3, "3 faces are required of this csg solid");
                    Assert.IsTrue(solid.Vertices.Count == 3, "3 vertices are required of this csg solid");
                    
                }
            }
        }

        [TestMethod]
        public void IfcHalfspace_Test()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var halfSpace = m.Instances.New<IfcHalfSpaceSolid>();
                    halfSpace.AgreementFlag = false;
                    var baseSurface = m.Instances.New<IfcPlane>();
                    baseSurface.Position = m.Instances.New<IfcAxis2Placement3D>();
                    baseSurface.Position.Location = m.Instances.New<IfcCartesianPoint>();
                    baseSurface.Position.Location.X = 0;
                    baseSurface.Position.Location.Y = 0;
                    baseSurface.Position.Location.Z = 10;
                    halfSpace.BaseSurface = baseSurface;
                    //make an extrusion
                    var profile = IfcModelBuilder.MakeRectangleHollowProfileDef(m, 20, 10, 1);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = geomEngine.CreateSolid(extrude, logger);
                    var halfSpaceSolid = geomEngine.CreateSolid(halfSpace, logger);
                    var cut = solid.Cut(halfSpaceSolid, 1e-5);
                    Assert.IsTrue(cut.Count > 0);
                    Assert.IsTrue(Math.Abs((solid.Volume * .25) - cut.First.Volume) < 1e-5);
                    //move the halfspace plane up
                    baseSurface.Position.Location.Z = 30;
                    halfSpaceSolid = geomEngine.CreateSolid(halfSpace, logger);
                    cut = solid.Cut(halfSpaceSolid, 1e-5);
                    Assert.IsTrue(Math.Abs((solid.Volume * .75) - cut.First.Volume) < 1e-5);
                    //reverse halfspace agreement
                    halfSpace.AgreementFlag = true;
                    halfSpaceSolid = geomEngine.CreateSolid(halfSpace, logger);
                    cut = solid.Cut(halfSpaceSolid, 1e-5);
                    Assert.IsTrue(Math.Abs((solid.Volume * .25) - cut.First.Volume) < 1e-5);
                   
                }
            }
        }

        [TestMethod]
        public void IfcPolygonalBoundedHalfspace_Test()
        {
            using (var m = new MemoryModel(new Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction(""))
                {
                    var polygonalBoundedHalfspace = m.Instances.New<IfcPolygonalBoundedHalfSpace>();
                    polygonalBoundedHalfspace.AgreementFlag = false;
                    var plane = m.Instances.New<IfcPlane>();
                    plane.Position = m.Instances.New<IfcAxis2Placement3D>();
                    plane.Position.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));

                    polygonalBoundedHalfspace.BaseSurface = plane;
                    //create the polygonal bound
                    var polyLine = m.Instances.New<IfcPolyline>();
                    polyLine.Points.Add(m.Instances.New<IfcCartesianPoint>(c => c.SetXY(0, 2.5)));
                    polyLine.Points.Add(m.Instances.New<IfcCartesianPoint>(c => c.SetXY(5, 2.5)));
                    polyLine.Points.Add(m.Instances.New<IfcCartesianPoint>(c => c.SetXY(5, -2.5)));
                    polyLine.Points.Add(m.Instances.New<IfcCartesianPoint>(c => c.SetXY(0, -2.5)));
                    polyLine.Points.Add(m.Instances.New<IfcCartesianPoint>(c => c.SetXY(0, 2.5)));
                    polygonalBoundedHalfspace.PolygonalBoundary = polyLine;

                    var basePos = m.Instances.New<IfcAxis2Placement3D>();
                    basePos.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
                    polygonalBoundedHalfspace.Position = basePos;
                    //make an extrusion
                    var profile = IfcModelBuilder.MakeRectangleProfileDef(m, 20, 10);
                    var extrude = IfcModelBuilder.MakeExtrudedAreaSolid(m, profile, 40);
                    var solid = geomEngine.CreateSolid(extrude, logger);
                    var halfSpaceSolid = geomEngine.CreateSolid(polygonalBoundedHalfspace, logger);
                    var cut = solid.Cut(halfSpaceSolid, 1e-5);

                    Assert.IsTrue(cut.Count > 0);
                    Assert.IsTrue(Math.Abs((solid.Volume) - cut.First.Volume - 1000) < 1e-5);

                    //reverse halfspace agreement
                    polygonalBoundedHalfspace.AgreementFlag = true;
                    halfSpaceSolid = geomEngine.CreateSolid(polygonalBoundedHalfspace, logger);
                    cut = solid.Cut(halfSpaceSolid, 1e-5);
                    Assert.IsTrue(Math.Abs(solid.Volume - cut.First.Volume) < 1e-5);

                    //move the plane up
                    plane.Position.Location.Z = 20;
                    halfSpaceSolid = geomEngine.CreateSolid(polygonalBoundedHalfspace, logger);
                    cut = solid.Cut(halfSpaceSolid, 1e-5);
                    Assert.IsTrue(Math.Abs(solid.Volume - cut.First.Volume - 500) < 1e-5);

                    //some realistic data
                    polyLine.Points[0].SetXY(0, 0);
                    polyLine.Points[1].SetXY(0, 2850);
                    polyLine.Points[2].SetXY(-350, 2850);
                    polyLine.Points[3].SetXY(-350, 0);
                    polyLine.Points[4].SetXY(0, 0);
                    plane.Position.Location.SetXYZ(-5240.7742616303667, -33052.9790707385, 0.0);
                    plane.Position.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(0, -1, 0));
                    plane.Position.RefDirection = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
                    basePos.Location.SetXYZ(-5240.7742616303667, -33052.9790707385, 0);
                    basePos.Axis = plane.Position.Axis;
                    basePos.RefDirection = plane.Position.RefDirection;

                    halfSpaceSolid = geomEngine.CreateSolid(polygonalBoundedHalfspace, logger);

                    profile.XDim = 350;
                    profile.YDim = 125;
                    profile.Position.Location.SetXY(-5415.7742616303676, -32932.529070738507);
                    extrude.Depth = 2850;
                    solid = geomEngine.CreateSolid(extrude, logger);

                    cut = solid.Cut(halfSpaceSolid, 1e-5); //everything should be cut
                    Assert.IsTrue(cut.Count == 0);
                   
                }
            }
        }


        [TestMethod]
        public void IfcHalfspaceCutFromIfcExtrudedAreaSolidTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(IfcHalfspaceCutFromIfcExtrudedAreaSolidTest)))
            {
                var solid = geomEngine.CreateSolid(er.Entity,logger);
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");
            }
        }


        [TestMethod]
        public void IfcPolygonalBoundedHalfspaceCutFromIfcExtrudedAreaSolidTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(IfcPolygonalBoundedHalfspaceCutFromIfcExtrudedAreaSolidTest)))
            {
                var solid = geomEngine.CreateSolid(er.Entity,logger);
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");
            }
        }

        [TestMethod]
        public void NestedBooleansTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(NestedBooleansTest)))
            {
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");
            }
        }

        [TestMethod]
        public void NestedBooleanClippingResultsTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(NestedBooleanClippingResultsTest)))
            {
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 7, "This solid should have 7 faces");
            }
        }

        [TestMethod]
        public void EmptyBooleanClippingResultTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(EmptyBooleanClippingResultTest)))
            {
                var solid = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsFalse(solid.Faces.Any(), "This solid should have 0 faces");
            }
        }
        [TestMethod]
        public void CuttingOpeningInIfcFaceBasedSurfaceModelTest()
        {
            using (var bodyEntity = new EntityRepository<IIfcFaceBasedSurfaceModel>("CuttingOpeningInIfcFaceBasedSurfaceModelBodyTest"))
            {
                using (var holeEntity = new EntityRepository<IIfcExtrudedAreaSolid>("CuttingOpeningInIfcFaceBasedSurfaceModelVoidTest"))
                {
                    var body = geomEngine.CreateSolidSet(bodyEntity.Entity, logger);
                    var hole = geomEngine.CreateSolid(holeEntity.Entity, logger);
                    var result = body.Cut(hole,bodyEntity.Entity.Model.ModelFactors.Precision);
                    Assert.IsTrue(result.Count == 2, "Two solids should be returned");
                    foreach (var solid in result)
                    {
                        IsSolidTest(solid);
                    }
                   
                } 
               
            }
        }   

        //        [TestMethod]
        //        public void Faceted_BRep_With_Void()
        //        {
        //            using (var eventTrace = LoggerFactory.CreateEventTrace())
        //            {
        //                using (var m = IfcStore.Open("SolidTestFiles\\Faceted BRep with void.ifc"))
        //                {
        //                    var wallShape = m.Instances[33] as IIfcFacetedBrep;
        //                    Assert.IsTrue(wallShape != null, "No IfcFacetedBrep found");
        //                    var windowShape = m.Instances[334] as IIfcExtrudedAreaSolid;
        //                    Assert.IsTrue(windowShape != null, "No IfcExtrudedAreaSolid found");
        //                    var windowOpening = m.Instances[328] as IIfcOpeningElement;
        //                    var shapes = _xbimGeometryCreator.CreateSolidSet(wallShape);
        //                    Assert.IsNotNull(windowOpening);
        //                    var hole =
        //                        (IXbimSolid)
        //                            _xbimGeometryCreator.Create(windowShape,
        //                                (IIfcAxis2Placement3D)
        //                                    ((IIfcLocalPlacement)windowOpening.ObjectPlacement).RelativePlacement);
        //                    var result = shapes.Cut(hole, m.ModelFactors.Precision);
        //                    double shapesSurfaceArea = 0;
        //                    foreach (var item in shapes)
        //                    {
        //                        shapesSurfaceArea += item.SurfaceArea;
        //                    }
        //                    double resultSurfaceArea = 0;
        //                    foreach (var item in result)
        //                    {
        //                        resultSurfaceArea += item.SurfaceArea;
        //                    }
        //                    Assert.IsTrue(Math.Abs(resultSurfaceArea - shapesSurfaceArea) < 1e-5); //nothing should have happened as the opening is already in the faceted brep
        //                    Assert.IsTrue(eventTrace.Events.Count == 1); //one event should have been raised from this call as a face is discarded from an illegal geometry
        //                }
        //            }
        //        }
        //        [TestMethod]
        //        public void Boolean_With_BoxedHalfSpace()
        //        {
        //            using (var eventTrace = LoggerFactory.CreateEventTrace())
        //            {
        //                using (var m = IfcStore.Open("SolidTestFiles\\10- Boxed Half Space.ifc"))
        //                {
        //                    var eas = m.Instances[28] as IIfcBooleanClippingResult;
        //                    Assert.IsTrue(eas != null, "No IfcBooleanClippingResult found");
        //                    Assert.IsTrue(eas.SecondOperand is IIfcBoxedHalfSpace, "Incorrect second operand found");

        //                    var solid = _xbimGeometryCreator.CreateSolid(eas);
        //                    Assert.IsTrue(eventTrace.Events.Count == 0); //no events should have been raised from this call
        //                    IfcCsgTests.GeneralTest(solid);
        //                    //  var w = new XbimOccWriter();
        //                    //  w.Write(solid, "d:\\xbim\\s");
        //                    Assert.IsTrue(solid.Faces.Count() == 8, "This solid should have 8 faces");
        //                }
        //            }
        //        }
        //#if USE_CARVE_CSG

        //        #region Mixed cut tests

        //		[TestMethod]
        //        public void BooleanCutSolidWithFacetedSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanCutSolidWithFacetedSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);

        //                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Cut(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanCutFacetedSolidWithFacetedSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Cut(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanCutFacetedSolidWithFacetedSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanCutFacetedSolidWithSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Cut(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanCutFacetedSolidWithSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Cut(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsFalse(solidSet.First.IsPolyhedron, "This should return a shape with curves");
        //                    Assert.IsTrue(solidSet.Count == 1, "Cutting these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        } 
        //        #endregion




        //        #region Mixed union tests

        //        [TestMethod]
        //        public void BooleanUnionSolidWithFacetedSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Union(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);
        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanUnionSolidWithFacetedSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);

        //                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Union(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanUnionFacetedSolidWithFacetedSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Union(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanUnionFacetedSolidWithFacetedSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Union(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanUnionFacetedSolidWithSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Union(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanUnionFacetedSolidWithSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Union(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsFalse(solidSet.First.IsPolyhedron, "This should return a shape with curves");
        //                    Assert.IsTrue(solidSet.Count == 1, "Unioning these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }
        //        #endregion

        //        #region Mixed intersection tests

        //        [TestMethod]
        //        public void BooleanIntersectSolidWithFacetedSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Intersection(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);
        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanIntersectSolidWithFacetedSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);

        //                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Intersection(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanIntersectFacetedSolidWithFacetedSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    small = XbimGeometryCreator.CreateFacetedSolid(small, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Intersection(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanIntersectFacetedSolidWithFacetedSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    inner = XbimGeometryCreator.CreateFacetedSolid(inner, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Intersection(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanIntersectFacetedSolidWithSolidPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 10, 15);

        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small = XbimGeometryCreator.CreateSolid(block2);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = big.Intersection(small, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanIntersectFacetedSolidWithSolidNonPlanarTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var cylinderInner = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    var cylinderOuter = IfcModelBuilder.MakeRightCircularCylinder(m, 20, 20);

        //                    var outer = XbimGeometryCreator.CreateSolid(cylinderOuter);
        //                    var inner = XbimGeometryCreator.CreateSolid(cylinderInner);
        //                    outer = XbimGeometryCreator.CreateFacetedSolid(outer, m.ModelFactors.Precision,
        //                        m.ModelFactors.DeflectionTolerance);
        //                    var result = outer.Intersection(inner, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsFalse(solidSet.First.IsPolyhedron, "This should return a shape with curves");
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }
        //        #endregion

        //#endif
        //        #region Compound booleans

        //        [TestMethod]
        //        public void BooleanCutMultipleSolidsFromASinglePlanarSolidTest()
        //        {
        //            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
        //                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
        //                    block3.Position.Location.X += 3;
        //                    block3.Position.Location.Y += 3;
        //                    var big = _xbimGeometryCreator.CreateSolid(block1);
        //                    var small1 = _xbimGeometryCreator.CreateSolid(block2);
        //                    var small2 = _xbimGeometryCreator.CreateSolid(block3);
        //                    var solids = _xbimGeometryCreator.CreateSolidSet();
        //                    solids.Add(small1);
        //                    solids.Add(small2);
        //                    var solidSet = big.Cut(solids, m.ModelFactors.PrecisionBoolean);

        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);
        //                    txn.Commit();
        //                }
        //            }
        //        }

        //#if USE_CARVE_CSG
        //        [TestMethod]
        //        public void BooleanCutMultipleSolidsFromASingleFacetedPlanarSolidTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
        //                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
        //                    block3.Position.Location.X += 3;
        //                    block3.Position.Location.Y += 3;
        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    big = XbimGeometryCreator.CreateFacetedSolid(big, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
        //                    var small1 = XbimGeometryCreator.CreateSolid(block2);
        //                    var small2 = XbimGeometryCreator.CreateSolid(block3);
        //                    var solids = XbimGeometryCreator.CreateSolidSet();
        //                    solids.Add(small1);
        //                    solids.Add(small2);
        //                    var result = big.Cut(solids, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        }

        //#endif
        //        [TestMethod]
        //        public void BooleanCutMultipleSolidsFromASingleNonPlanarSolidTest()
        //        {
        //            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 15);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
        //                    block2.Position.Location.X += 6;
        //                    block2.Position.Location.Y += 6;
        //                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
        //                    block3.Position.Location.X += 3;
        //                    block3.Position.Location.Y += 3;
        //                    var big = _xbimGeometryCreator.CreateSolid(block1);
        //                    var small1 = _xbimGeometryCreator.CreateSolid(block2);
        //                    var small2 = _xbimGeometryCreator.CreateSolid(block3);
        //                    var solids = _xbimGeometryCreator.CreateSolidSet();
        //                    solids.Add(small1);
        //                    solids.Add(small2);
        //                    var solidSet = big.Cut(solids, m.ModelFactors.PrecisionBoolean);

        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);
        //                    txn.Commit();
        //                }
        //            }
        //        }

        //#if USE_CARVE_CSG
        //        [TestMethod]
        //        public void BooleanCutMultipleFacetedSolidsFromASingleNonPlanarSolidTest()
        //        {
        //            using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 15);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 1, 2, 10);
        //                    block2.Position.Location.X += 6;
        //                    block2.Position.Location.Y += 6;
        //                    var block3 = IfcModelBuilder.MakeBlock(m, 1, 2, 15);
        //                    block3.Position.Location.X += 3;
        //                    block3.Position.Location.Y += 3;
        //                    var big = XbimGeometryCreator.CreateSolid(block1);
        //                    var small1 = XbimGeometryCreator.CreateSolid(block2);
        //                    var small2 = XbimGeometryCreator.CreateSolid(block3);
        //                    small1 = XbimGeometryCreator.CreateFacetedSolid(small1, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
        //                    small2 = XbimGeometryCreator.CreateFacetedSolid(small2, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance);
        //                    var solids = XbimGeometryCreator.CreateSolidSet();
        //                    solids.Add(small1);
        //                    solids.Add(small2);
        //                    var result = big.Cut(solids, m.ModelFactors.PrecisionBoolean);
        //                    var solidSet = (IXbimSolidSet)result;
        //                    Assert.IsTrue(solidSet.Count == 1, "Intersection of these two solids should return a single solid");
        //                    IfcCsgTests.GeneralTest(solidSet.First);

        //                }
        //            }
        //        } 
        //#endif

        //        #endregion
        //        #region Single solid from multiple solids

        //        [TestMethod]
        //        public void BooleanCutSingleSolidsFromAMultipleNonPlanarSolidTest()
        //        {
        //            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
        //                    block2.Position.Location.Z += 22; //stack block2 2 above block1
        //                    var block3 = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
        //                    block3.Position.Location.Z += 10; //in bretween 1 and 2
        //                    var b1 = _xbimGeometryCreator.CreateSolid(block1);
        //                    var b2 = _xbimGeometryCreator.CreateSolid(block2);
        //                    var b3 = _xbimGeometryCreator.CreateSolid(block3);
        //                    var solids = _xbimGeometryCreator.CreateSolidSet();
        //                    solids.Add(b1);
        //                    solids.Add(b2);
        //                    var result = solids.Cut(b3, m.ModelFactors.PrecisionBoolean);
        //                    Assert.IsTrue(result.Count == 2, "Cutting of these two solids should return two  solids");

        //                    foreach (var solid in result)
        //                        IfcCsgTests.GeneralTest(solid);
        //                    txn.Commit();
        //                }
        //            }
        //        }

        //#if USE_CARVE_CSG
        //        [TestMethod]
        //        public void BooleanCutSingleFacetedSolidsFromMultipleNonPlanarSolidsTest()
        //        {
        //           using (var m = XbimModel.CreateTemporaryModel())
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 15, 20, 20);
        //                    block2.Position.Location.Z += 22; //stack block2 2 above block1
        //                    var block3 = IfcModelBuilder.MakeRightCircularCylinder(m, 10, 20);
        //                    block3.Position.Location.Z += 10; //in bretween 1 and 2
        //                    var b1 = XbimGeometryCreator.CreateSolid(block1);
        //                    var b2 = XbimGeometryCreator.CreateSolid(block2);
        //                    var b3 = XbimGeometryCreator.CreateSolid(block3);
        //                    var solids = XbimGeometryCreator.CreateSolidSet();
        //                    solids.Add(b1);
        //                    solids.Add(b2);
        //                    var result = solids.Cut(b3, m.ModelFactors.PrecisionBoolean);

        //                    Assert.IsTrue(result.Count == 2, "Cutting of these two solids should return two  solids");

        //                    foreach (var solid in result)
        //                         IfcCsgTests.GeneralTest(solid);

        //                }
        //            }
        //        } 
        //#endif
        //        #endregion

        //        #region Solid with voids test
        //        [TestMethod]
        //        public void BooleanCutSolidWithVoidPlanarTest()
        //        {
        //            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeBlock(m, 20, 20, 20);
        //                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 5, 5);
        //                    block2.Position.Location.X += 10;
        //                    block2.Position.Location.Y += 10;
        //                    block2.Position.Location.Z += 10;

        //                    var b1 = _xbimGeometryCreator.CreateSolid(block1);
        //                    var b2 = _xbimGeometryCreator.CreateSolid(block2);
        //                    var result = b1.Cut(b2, m.ModelFactors.PrecisionBoolean);
        //                    Assert.IsTrue(result.Count == 1, "Cutting of these two solids should return two  solids");
        //                    foreach (var solid in result)
        //                        IfcCsgTests.GeneralTest(solid);
        //                    txn.Commit();
        //                }
        //            }
        //        }

        //        [TestMethod]
        //        public void BooleanCutSolidWithVoidNonPlanarTest()
        //        {
        //            using (var m = IfcStore.Create(new XbimEditorCredentials(), IfcSchemaVersion.Ifc4, XbimStoreType.InMemoryModel))
        //            {
        //                using (var txn = m.BeginTransaction())
        //                {

        //                    var block1 = IfcModelBuilder.MakeSphere(m, 20);
        //                    var block2 = IfcModelBuilder.MakeSphere(m, 5);

        //                    var b1 = _xbimGeometryCreator.CreateSolid(block1);
        //                    var b2 = _xbimGeometryCreator.CreateSolid(block2);
        //                    var result = b1.Cut(b2, m.ModelFactors.PrecisionBoolean);
        //                    Assert.IsTrue(result.Count == 1, "Cutting of these two solids should return two  solids");
        //                    const double vOuter = (4.0 / 3.0) * Math.PI * 20.0 * 20.0 * 20.0;
        //                    const double vInner = (4.0 / 3.0) * Math.PI * 5.0 * 5.0 * 5.0;
        //                    const double volume = vOuter - vInner;

        //                    Assert.IsTrue(result.First.Volume - volume <= m.ModelFactors.Precision, "Volume is incorrect");
        //                    txn.Commit();
        //                }
        //            }
        //        }
        //        #endregion
    }
}
