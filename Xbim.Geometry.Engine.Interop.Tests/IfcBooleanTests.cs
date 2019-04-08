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
using System.Collections.Generic;
using Xbim.Ifc.Extensions;

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
            loggerFactory = new LoggerFactory().AddDebug(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<IfcBooleanTests>();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }

        [TestMethod]
        public void SubtractionResultsInClosedWindow()
        {
            using (var model = MemoryModel.OpenRead(@".\TestFiles\SubtractionResultsInClosedWindow.ifc"))
            {
                logger.LogInformation("Running SubtractionResultsInClosedWindow");
                var wallBrep = model.Instances[12752] as IIfcFacetedBrep;
                var wallPlacement = model.Instances[12562] as IIfcLocalPlacement;
                var wallTransform = wallPlacement.ToMatrix3D();
                var openingExtrudeArea = model.Instances[286479] as IIfcExtrudedAreaSolid;
                var openingPlacement = model.Instances[286487] as IIfcLocalPlacement;
                var openingTransform = openingPlacement.ToMatrix3D();

                var wallSolid = geomEngine.CreateSolidSet(wallBrep, logger).FirstOrDefault();
                var transformedWall = wallSolid.Transform(wallTransform) as IXbimSolid;

                var openingSolid = geomEngine.CreateSolid(openingExtrudeArea, logger);
                var transformedOpening = openingSolid.Transform(openingTransform) as IXbimSolid;
                var cutWall = transformedWall.Cut(transformedOpening, model.ModelFactors.Precision, logger).FirstOrDefault();
                Assert.IsNotNull(cutWall, "Cut wall should not be null");
                // note this faceted brep already has the openings cut out and we are cutting them again so the volume should not change
                var volDiff = cutWall.Volume - transformedWall.Volume;
                Assert.IsTrue(Math.Abs(volDiff) < 1e-5);
               // Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
               //  var solid = geomEngine.CreateSolid(er.Entity, logger);
               //  Assert.IsFalse(solid.Faces.Any(), "This solid should have 0 faces");
            }

        }

        [TestMethod]
        public void CoordinationTest()
        {
            using (var erArch = new EntityRepository<IIfcSpace>("CoordinationTestArchitectureSpace"))
            {
                using (var erElec = new EntityRepository<IIfcSpace>("CoordinationTestElectricalSpace"))
                {
                    var archMatrix = erArch.Entity.ObjectPlacement.ToMatrix3D();
                    var elecMatrix = erElec.Entity.ObjectPlacement.ToMatrix3D();
                    var archRepItem = erArch.Entity.Representation.Representations.FirstOrDefault()?.Items.FirstOrDefault() as IIfcExtrudedAreaSolid;
                    var elecRepItem = erElec.Entity.Representation.Representations.FirstOrDefault()?.Items.FirstOrDefault() as IIfcExtrudedAreaSolid;
                    Assert.IsNotNull(archRepItem); Assert.IsNotNull(elecRepItem);
                    var archGeom = geomEngine.CreateSolid(archRepItem).Transform(archMatrix) ;
                    var elecGeom = geomEngine.CreateSolid(elecRepItem).Transform(elecMatrix) ;
                    var archBB = archGeom.BoundingBox;
                    var elecBB = elecGeom.BoundingBox;
                    var diff = archBB.Centroid() - elecBB.Centroid();
                    Assert.IsTrue(diff.Length<1e-5);
                }
            }

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
        [TestMethod]
        public void CompoundBooleanUnionTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(CompoundBooleanUnionTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                Assert.IsTrue(solids.Count == 1);

            }

        }
        /// <summary>
        /// Tests iIIfcBooleanResult that cuts two shape and leaves nothing
        /// </summary>
        [TestMethod]
        public void BooleanResultCompleteVoidCutTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(BooleanResultCompleteVoidCutTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                Assert.IsTrue(solids.Count == 0);

            }

        }
        [TestMethod]
        public void CsgBooleanResultTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(CsgBooleanResultTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                Assert.IsTrue(solids.Count == 2, "This should produce two solids");

            }

        }
        /// <summary>
        /// Tests if a boolean processes correctly if not it will silent fail and the test should fail
        /// </summary>
        [TestMethod]
        public void BooleanSilentFailTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(BooleanSilentFailTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());
                
            }

        }

 /// <summary>
        /// This problem is a boolean where the tolerance needs to be made courser by 10 fold
        /// </summary>
        [TestMethod]
        public void BadlyOrientedBrepFacesTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(BadlyOrientedBrepFacesTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());
                
            }

        }
        /// <summary>
        /// This problem is a boolean where the tolerance needs to be made courser by 10 fold
        /// </summary>
        [TestMethod]
        public void FaceWithBoundsOutsideDeclaredPrecisionTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(FaceWithBoundsOutsideDeclaredPrecisionTest)))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");
                
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());
                
            }

        }

        [TestMethod]
        public void UnorderdedCompositeCurveTest()
        {
            using (var er = new EntityRepository<IIfcCompositeCurve>(nameof(UnorderdedCompositeCurveTest),true))
            {
                Assert.IsTrue(er.Entity != null, "No IIfcCompositeCurve found");

                var wire = geomEngine.CreateWire(er.Entity, logger);
                Assert.IsNotNull(wire);
                Assert.IsTrue(wire.IsValid);

            }

        }
        [TestMethod]
        public void BooleanResultTimoutTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(BooleanResultTimoutTest), true))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");

                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());

            }

        }
        [TestMethod]
        public void ComplexNestedBooleanResult()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(ComplexNestedBooleanResult), true))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");

                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());

            }

        }
        [TestMethod]
        public void VerySmallBooleanCutTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(VerySmallBooleanCutTest), true))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");

                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());

            }

        }
        [TestMethod]
        public void FailingBooleanBrepWithZeroVolumeTest()
        {
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(FailingBooleanBrepWithZeroVolumeTest), true))
            {
                Assert.IsTrue(er.Entity != null, "No IfcBooleanResult found");

                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                HelperFunctions.IsValidSolid(solids.FirstOrDefault());

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
                var solids = geomEngine.CreateSolidSet(er.Entity, logger);
                Assert.IsTrue(solids.Count==0, "No solids should be produced");
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
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                Assert.IsTrue(solid.Faces.Count==6, "This solid should have 6 faces");
            }

        }

     

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

                    var solid = geomEngine.CreateSolidSet(csgTree,logger).FirstOrDefault();
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

                    var solid = geomEngine.CreateSolidSet(csgTree,logger).FirstOrDefault();
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

                    var solid = geomEngine.CreateSolidSet(csgTree,logger).FirstOrDefault();
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

                    var solid = geomEngine.CreateSolidSet(csgTree, logger).FirstOrDefault();
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
                var solid = geomEngine.CreateSolidSet(er.Entity,logger).FirstOrDefault();
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");
            }
        }


        [TestMethod]
        public void IfcPolygonalBoundedHalfspaceCutFromIfcExtrudedAreaSolidTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(IfcPolygonalBoundedHalfspaceCutFromIfcExtrudedAreaSolidTest)))
            {
                var solid = geomEngine.CreateSolidSet(er.Entity,logger).FirstOrDefault();
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");
            }
        }

        [TestMethod]
        public void NestedBooleansTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(NestedBooleansTest)))
            {
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 6, "Rectangular profiles should have six faces");
            }
        }

        [TestMethod]
        public void NestedBooleanClippingResultsTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(NestedBooleanClippingResultsTest)))
            {
                var solid = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                IsSolidTest(solid);
                Assert.IsTrue(solid.Faces.Count() == 7, "This solid should have 7 faces");
            }
        }

        //this test is 2 boolean clipping on efffectivel a beam, but the second cut is an illegal solid with coincidental faces
        //this test checks that the booleans do the right thing
        [TestMethod]
        public void SmallBooleanClippingResultsTest()
        {
            IXbimSolid solidBody, solidCut1, solidCut2, solidResult;
            // this is a simple cuboid / beam
            using (var er = new EntityRepository<IIfcExtrudedAreaSolid>("SmallBooleanClippingResultsTestBodyShape"))
            {
                solidBody = geomEngine.CreateSolid(er.Entity, logger);
                Assert.IsTrue(solidBody.Faces.Count==6, "This solid should have 6 faces");
            }
            //this is a triangular fillet (prism) to cut off the sort side face
            using (var er = new EntityRepository<IIfcFacetedBrep>("SmallBooleanClippingResultsTestCutShape1"))
            {
                solidCut1 = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                Assert.IsTrue(solidCut1.Faces.Count == 5, "This solid should have 5 faces");
            }
            // this shape is a faulty solid
            using (var er = new EntityRepository<IIfcFacetedBrep>("SmallBooleanClippingResultsTestCutShape2"))
            {
                solidCut2 = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                Assert.IsTrue(solidCut2.Faces.Count == 10, "This solid should have 10 faces");
            }
            using (var er = new EntityRepository<IIfcBooleanResult>(nameof(SmallBooleanClippingResultsTest)))
            {
                solidResult = geomEngine.CreateSolidSet(er.Entity, logger).FirstOrDefault();
                var actualVolume = solidResult.Volume;
                Assert.IsTrue(solidBody.Volume > actualVolume, "This cut solid should have less volume than the body shape");
                Assert.IsTrue(solidResult.Faces.Count == 10, "This solid should have 10 faces");
            }
        }
        [TestMethod]
        public void EmptyBooleanClippingResultTest()
        {
            using (var er = new EntityRepository<IIfcBooleanClippingResult>(nameof(EmptyBooleanClippingResultTest)))
            {
                var solidSet = geomEngine.CreateSolidSet(er.Entity, logger);
                Assert.IsTrue(solidSet.Count==0, "This solid should be empty");
            }
        }



        [TestMethod]
        public void CuttingOpeningInCompositeProfileDefTest()
        {
            using (var repos = new EntityRepository<IIfcRelVoidsElement>("CuttingOpeningInCompositeProfileDefTest"))
            {
                
                var relVoids =  repos.Entity;
                var oneMilli = relVoids.Model.ModelFactors.OneMilliMetre;
                var precision = relVoids.Model.ModelFactors.Precision + (oneMilli * 2e-6);
                var wall = relVoids.RelatingBuildingElement;
                var wallPlacement = wall.ObjectPlacement as IIfcLocalPlacement;
                var wallTransform = wallPlacement.ToMatrix3D();
                var wallTransform2 = geomEngine.ToMatrix3D(wallPlacement);
                Assert.AreEqual(wallTransform, wallTransform2);
                var wallGeom = wall.Representation.Representations.FirstOrDefault().Items.FirstOrDefault()  as IIfcExtrudedAreaSolid;
                var opening = relVoids.RelatedOpeningElement;
                var openingPlacement = opening.ObjectPlacement as IIfcLocalPlacement;
                var openingGeoms = opening.Representation.Representations.FirstOrDefault().Items.OfType<IIfcExtrudedAreaSolid>().ToList();
                var openingTransform = openingPlacement.ToMatrix3D(); ;
                var wallBrep = geomEngine.CreateSolidSet(wallGeom);
                var wallBrepPlaced = wallBrep.Transform(wallTransform) as IXbimSolidSet;

                var openingBReps = geomEngine.CreateSolidSet();
                foreach (var og in openingGeoms)
                {
                    var brep = geomEngine.CreateSolid(og);
                    openingBReps.Add(brep.Transform(openingTransform) as IXbimSolid);
                }
                int uncut = 0;
                var singleCut = wallBrepPlaced.Cut(openingBReps, precision);
                var vol = 0.0;
                foreach (var uncutItem in wallBrepPlaced) 
                {
                    var result = uncutItem.Cut(openingBReps, precision);
                    Assert.IsTrue(result.Count == 1);
                    var cutSolid = result.First as IXbimSolid;
                    Assert.IsNotNull(cutSolid);
                    Assert.IsTrue(cutSolid.IsValid);
                    if (uncutItem.Volume <= cutSolid.Volume) uncut++;
                    Assert.IsTrue(uncut<=3, "More than two solids are uncut, there should only be two");
                    vol += cutSolid.Volume;
                }
                Assert.IsTrue(uncut == 2);
                var scutVol = singleCut.Sum(s => s.Volume);
                Assert.IsTrue(Math.Abs(vol- scutVol) <1e-5);
                

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
                    Assert.IsTrue(body.Count == 8, "Eight solids should be returned");
                    var hole = geomEngine.CreateSolid(holeEntity.Entity, logger);
                    var result = body.Cut(hole,bodyEntity.Entity.Model.ModelFactors.Precision);

                    Assert.IsTrue(result.Count == 8, "Eight solids should be returned");
                    foreach (var solid in result)
                    {
                        IsSolidTest(solid);
                    }

                } 
               
            }
        }

        #region Solid with voids test
        [TestMethod]
        public void BooleanCutSolidWithVoidPlanarTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Populate"))
                {

                    var block1 = IfcModelBuilder.MakeBlock(m, 20, 20, 20);
                    var block2 = IfcModelBuilder.MakeBlock(m, 5, 5, 5);
                    block2.Position.Location.X += 10;
                    block2.Position.Location.Y += 10;
                    block2.Position.Location.Z += 10;

                    var b1 = geomEngine.CreateSolid(block1);
                    var b2 = geomEngine.CreateSolid(block2);
                    var result = b1.Cut(b2, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(result.Count == 1, "Cutting of these two solids should return two  solids");
                    foreach (var solid in result)
                        HelperFunctions.IsValidSolid(solid);
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        public void BooleanCutSolidWithVoidNonPlanarTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Populate"))
                {

                    var block1 = IfcModelBuilder.MakeSphere(m, 20);
                    var block2 = IfcModelBuilder.MakeSphere(m, 5);

                    var b1 = geomEngine.CreateSolid(block1);
                    var b2 = geomEngine.CreateSolid(block2);
                    var result = b1.Cut(b2, m.ModelFactors.PrecisionBoolean);
                    Assert.IsTrue(result.Count == 1, "Cutting of these two solids should return two  solids");
                    const double vOuter = (4.0 / 3.0) * Math.PI * 20.0 * 20.0 * 20.0;
                    const double vInner = (4.0 / 3.0) * Math.PI * 5.0 * 5.0 * 5.0;
                    const double volume = vOuter - vInner;

                    Assert.IsTrue(result.First.Volume - volume <= m.ModelFactors.Precision, "Volume is incorrect");
                    txn.Commit();
                }
            }
        }
        #endregion

    }
}
