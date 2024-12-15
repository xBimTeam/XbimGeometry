﻿using System;
using System.Diagnostics;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Common.Geometry;
using Xbim.Common.Step21;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    
    [TestClass]
    
    public class IfcCsgTests
    {
        static private IXbimGeometryEngine geomEngine;
        static private ILoggerFactory loggerFactory;
        static private ILogger logger;

        [ClassInitialize]
        static public void Initialise(TestContext context)
        {
            loggerFactory = new LoggerFactory().AddConsole(LogLevel.Trace);
            geomEngine = new XbimGeometryEngine();
            logger = loggerFactory.CreateLogger<IfcCsgTests>();
        }
        [ClassCleanup]
        static public void Cleanup()
        {
            loggerFactory = null;
            geomEngine = null;
            logger = null;
        }
        [TestMethod]
        public void IfcRectangularPyramidTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var pyramid = m.Instances.New<IfcRectangularPyramid>();
                    var p = m.Instances.New<IfcAxis2Placement3D>();
                    p.Axis = m.Instances.New<IfcDirection>(d =>d.SetXYZ(1, 0, 0));
                    p.Location = m.Instances.New< IfcCartesianPoint>(c => c.SetXYZ(10, 10, 0));
                    pyramid.Position = p;
                    pyramid.Height = 20;
                    pyramid.XLength = 10;
                    pyramid.YLength = 15;
                    
                    var solid = geomEngine.CreateSolid(pyramid);
               
                    Assert.IsTrue(solid.Faces.Count == 5, "5 faces are required of a pyramid");
                    Assert.IsTrue(solid.Vertices.Count == 5, "5 vertices are required of a pyramid");
                    var meshRec = new MeshHelper();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    Assert.IsTrue(meshRec.FaceCount == 5, "5 mesh faces are required of a pyramid");
                    Assert.IsTrue(meshRec.PointCount == 16, "16 mesh points are required of a pyramid");
                    txn.Commit();
                }
            }
        }

        [TestMethod]
        [Ignore("Broken in 5.1. Reinstate in v6")]
        public void IfcRightCircularCylinderTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    const double h = 2; const double r = 0.5;             
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, r, h);
                    
                    var solid = geomEngine.CreateSolid(cylinder);
                   
                    Assert.IsTrue(solid.Faces.Count == 3, "3 faces are required of a cylinder");
                    Assert.IsTrue(solid.Vertices.Count == 2, "2 vertices are required of a cylinder");
                    var meshRec = new MeshHelper();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    Assert.IsTrue(meshRec.FaceCount == 3, "3 mesh faces are required of a cylinder");
                    Assert.IsTrue(meshRec.PointCount == 18, "18 mesh points are required of a cylinder");
                    txn.Commit();
                }
            }
        }

       

        [TestMethod]
        [Ignore("Broken in 5.1. Reinstate in v6")]
        public void IfcRightCircularConeTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var cylinder = m.Instances.New<IfcRightCircularCone>();
                    var p = m.Instances.New<IfcAxis2Placement3D>();
                    p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
                    p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0,0, 0));
                    cylinder.Position = p;
                    cylinder.BottomRadius = 0.5;
                    cylinder.Height = 2;
                    
                    var solid = geomEngine.CreateSolid(cylinder);
                    
                    Assert.IsTrue(solid.Faces.Count == 2, "2 faces are required of a cone");
                    Assert.IsTrue(solid.Vertices.Count == 2, "2 vertices are required of a cone");
                    var meshRec = new MeshHelper();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    Assert.IsTrue(meshRec.FaceCount == 2, "2 mesh faces are required of a cone");
                    Assert.IsTrue(meshRec.PointCount == 23, "23 mesh points are required of a cone");
                   txn.Commit();
                }
            }
        }

        [TestMethod]
        public void IfcBlockTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {

                    var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    
                    var solid = geomEngine.CreateSolid(block);
                    
                    Assert.IsTrue(solid.Faces.Count == 6, "6 faces are required of a block");
                    Assert.IsTrue(solid.Vertices.Count == 8, "8 vertices are required of a block");
                    var meshRec = new MeshHelper();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    Assert.IsTrue(meshRec.FaceCount == 6,  "6 mesh faces are required of a block");
                    Assert.IsTrue(meshRec.PointCount == 24, "24 mesh points are required of a block");
                   txn.Commit();
                }
            }
        }

       

      

        [TestMethod]
        [Ignore("Broken in 5.1. Reinstate in v6")]
        public void IfcSphereTest()
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    const double r = 0.5;
                    
                    var sphere = IfcModelBuilder.MakeSphere(m, r);
                    
                    var solid = geomEngine.CreateSolid(sphere);
                    Assert.IsTrue(solid.Faces.Count == 1, "1 face is required of a sphere");
                    Assert.IsTrue(solid.Vertices.Count == 2, "2 vertices are required of a sphere");
                    var meshRec = new MeshHelper();
                    meshRec.BeginUpdate();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    meshRec.EndUpdate();
                    Assert.IsTrue(meshRec.FaceCount == 1, "1 mesh face is required of a sphere");
                    Assert.IsTrue(meshRec.PointCount == 19, "19 mesh points are required of a sphere");
                    Assert.IsTrue(meshRec.TriangleCount == 28, "28 triangles are required of a sphere");
                    Assert.IsTrue(meshRec.TriangleCount*3 == meshRec.TriangleIndicesCount,"Incorrect triangulation");
                    txn.Commit();
                }
            }
        }

        public static void GeneralTest(IXbimSolid solid, bool ignoreVolume = false, bool isHalfSpace = false, int entityLabel = 0)
        {
            if (ignoreVolume && !isHalfSpace && solid.Volume == 0)
            {
                Trace.WriteLine(String.Format("Entity  #{0} has zero volume>", entityLabel));
            }
            if (!ignoreVolume)
                Assert.IsTrue(solid.Volume > 0, "Volume should be greater than 0");
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
                    {
                        Assert.IsFalse(face.OuterBound.IsPlanar, "Face is not planar but wire is planar in #" + entityLabel);
                    }

                    foreach (var edge in face.OuterBound.Edges)
                    {
                        Assert.IsTrue(edge.EdgeGeometry.IsValid, "Edge element is invalid in #" + entityLabel);
                        Assert.IsTrue(edge.EdgeStart.IsValid, "Edge start is invalid in #" + entityLabel);
                        Assert.IsTrue(edge.EdgeEnd.IsValid, "Edge end is invalid in #" + entityLabel);
                    }
                }
            }
        }


    }
}
