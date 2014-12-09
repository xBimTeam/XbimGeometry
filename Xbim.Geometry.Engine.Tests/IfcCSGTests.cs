using System;
using System.Diagnostics;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc2x3.GeometricModelResource;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.IO;
using XbimGeometry.Interfaces;

namespace GeometryTests
{
    [TestClass]
    [DeploymentItem(@"SolidTestFiles\")]
    [DeploymentItem("Xbim.Geometry.Engine32.dll")]
    [DeploymentItem("Xbim.Geometry.Engine64.dll")]
    public class IfcCsgTests
    { 

        private readonly XbimGeometryEngine _xbimGeometryCreator = new XbimGeometryEngine();
        [TestMethod]
        public void IfcRectangularPyramidTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    var pyramid = m.Instances.New<IfcRectangularPyramid>();
                    var p = m.Instances.New<IfcAxis2Placement3D>();
                    p.Axis = m.Instances.New<IfcDirection>(d =>d.SetXYZ(1, 0, 0));
                    p.Location = m.Instances.New< IfcCartesianPoint>(c => c.SetXYZ(10, 10, 0));
                    pyramid.Position = p;
                    pyramid.Height = 20;
                    pyramid.XLength = 10;
                    pyramid.YLength = 15;
                    
                    var solid = _xbimGeometryCreator.CreateSolid(pyramid);
               
                    Assert.IsTrue(solid.Faces.Count == 5, "5 faces are required of a pyramid");
                    Assert.IsTrue(solid.Vertices.Count == 5, "5 vertices are required of a pyramid");
                }
            }
        }

        [TestMethod]
        public void IfcRightCircularCylinderTest()
        {
            using (var m = XbimModel.CreateTemporaryModel()) 
            {
                using (var txn = m.BeginTransaction())
                {
                    const double h = 20; const double r = 10;             
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, r, h);
                    
                    var solid = _xbimGeometryCreator.CreateSolid(cylinder);
                   
                    Assert.IsTrue(solid.Faces.Count == 3, "3 faces are required of a cylinder");
                    Assert.IsTrue(solid.Vertices.Count == 2, "2 vertices are required of a cylinder");

                }
            }
        }

       

        [TestMethod]
        public void IfcRightCircularConeTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    var cylinder = m.Instances.New<IfcRightCircularCone>();
                    var p = m.Instances.New<IfcAxis2Placement3D>();
                    p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
                    p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(10, 10, 0));
                    cylinder.Position = p;
                    cylinder.BottomRadius = 10;
                    cylinder.Height = 20;
                    
                    var solid = _xbimGeometryCreator.CreateSolid(cylinder);
                    
                    Assert.IsTrue(solid.Faces.Count == 2, "2 faces are required of a cone");
                    Assert.IsTrue(solid.Vertices.Count == 2, "2 vertices are required of a cone");
                   
                }
            }
        }

        [TestMethod]
        public void IfcBlockTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {

                    var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    
                    var solid = _xbimGeometryCreator.CreateSolid(block);
                    
                    Assert.IsTrue(solid.Faces.Count == 6, "6 faces are required of a block");
                    Assert.IsTrue(solid.Vertices.Count == 8, "8 vertices are required of a block");
                   
                }
            }
        }

       

      

        [TestMethod]
        public void IfcSphereTest()
        {
            using (var m = XbimModel.CreateTemporaryModel())
            {
                using (var txn = m.BeginTransaction())
                {
                    const double r = 10;
                    
                    var sphere = IfcModelBuilder.MakeSphere(m, r);
                    
                    var solid = _xbimGeometryCreator.CreateSolid(sphere);
                    Assert.IsTrue(solid.Faces.Count == 1, "1 face is required of a sphere");
                    Assert.IsTrue(solid.Vertices.Count == 2, "2 vertices are required of a sphere");
                }
            }
        }

       

        public static void GeneralTest(IXbimSolid solid, bool ignoreVolume = false, bool isHalfSpace= false, int entityLabel = 0)
        {
            if (ignoreVolume && !isHalfSpace && solid.Volume == 0)
            {
                Trace.WriteLine(String.Format("Entity  #{0} has zero volume>", entityLabel));
            }
            if(!ignoreVolume) Assert.IsTrue(solid.Volume > 0, "Volume should be greater than 0");
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
    }
}
