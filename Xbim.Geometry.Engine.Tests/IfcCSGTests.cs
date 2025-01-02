using System;
using System.Diagnostics;
using FluentAssertions;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Abstractions;
using Xbim.Common.Geometry;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Geometry.Engine.Tests;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

  
    public class IfcCsgTests
    {

        private readonly ILogger _logger;
       
        private readonly IXbimGeometryServicesFactory factory;
        private readonly ILoggerFactory _loggerFactory;

        public IfcCsgTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            this.factory = factory;
            _loggerFactory = loggerFactory;
            _logger = _loggerFactory.CreateLogger<IfcCsgTests>();
        }
        
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void IfcRectangularPyramidTest(XGeometryEngineVersion engineVersion)
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var pyramid = m.Instances.New<IfcRectangularPyramid>();
                    var p = m.Instances.New<IfcAxis2Placement3D>();
                    p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
                    p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(10, 10, 0));
                    pyramid.Position = p;
                    pyramid.Height = 20;
                    pyramid.XLength = 10;
                    pyramid.YLength = 15;
                    var geomEngineV5 = factory.CreateGeometryEngine(engineVersion, m, _loggerFactory);
                   
                    var solid = geomEngineV5.CreateSolid(pyramid);
                    
                    solid.Shells.Count.Should().Be(1);
                    solid.Faces.Count.Should().Be(5, "5 faces are required of a pyramid");
                    solid.Vertices.Count.Should().Be(5, "5 vertices are required of a pyramid");
                    //var meshRec = new MeshHelper();
                    //geomEngineV5.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);

                    //meshRec.FaceCount == 5, "5 mesh faces are required of a pyramid");
                    //meshRec.PointCount == 16, "16 mesh points are required of a pyramid");

                    txn.Commit();
                }
            }
        }
        
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void IfcRightCircularCylinderTest(XGeometryEngineVersion engineVersion)
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    const double h = 2; const double r = 0.5;
                    var cylinder = IfcModelBuilder.MakeRightCircularCylinder(m, r, h);
                    var geomEngineV5 = factory.CreateGeometryEngine(engineVersion, m, _loggerFactory);
                    var solid = geomEngineV5.CreateSolid(cylinder, _logger);

                    solid.Faces.Count.Should().Be(3, "3 faces are required of a cylinder");
                    solid.Vertices.Count.Should().Be(2,"2 vertices are required of a cylinder");
                    var meshRec = new MeshHelper();
                    geomEngineV5.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    meshRec.FaceCount.Should().Be(3, "3 mesh faces are required of a cylinder");
                    meshRec.PointCount.Should().Be(18, "18 mesh points are required of a cylinder");
                    txn.Commit();
                }
            }
        }
        

        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void IfcRightCircularConeTest(XGeometryEngineVersion engineVersion)
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    var cylinder = m.Instances.New<IfcRightCircularCone>();
                    var p = m.Instances.New<IfcAxis2Placement3D>();
                    p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
                    p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
                    cylinder.Position = p;
                    cylinder.BottomRadius = 0.5;
                    cylinder.Height = 2;
                    var geomEngine = factory.CreateGeometryEngine(engineVersion, m, _loggerFactory);
                    var solid = geomEngine.CreateSolid(cylinder, _logger);

                    solid.Faces.Count.Should().Be(2, "2 faces are required of a cone");
                    solid.Vertices.Count.Should().Be(2, "2 vertices are required of a cone");
                    var meshRec = new MeshHelper();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    meshRec.FaceCount.Should().Be(2, "2 mesh faces are required of a cone");
                    meshRec.PointCount.Should().Be(23, "23 mesh points are required of a cone");
                    txn.Commit();
                }
            }
        }

       
        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void IfcBlockTest(XGeometryEngineVersion engineVersion)
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {

                    var block = IfcModelBuilder.MakeBlock(m, 10, 15, 20);
                    try
                    {

                    var geomEngine = factory.CreateGeometryEngine(engineVersion, m, _loggerFactory);
                    var solid = geomEngine.CreateSolid(block, _logger);

                    solid.Faces.Count.Should().Be(6, "6 faces are required of a block");
                    solid.Vertices.Count.Should().Be(8, "8 vertices are required of a block");
                    var meshRec = new MeshHelper();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    meshRec.FaceCount.Should().Be(6, "6 mesh faces are required of a block");
                    meshRec.PointCount.Should().Be(24, "24 mesh points are required of a block");
                    txn.Commit();

                    }
                    catch (Exception e)
                    {
                        var ms = e.Message;
                        throw;
                    }
                }
            }
        }



        [Theory]
        [InlineData(XGeometryEngineVersion.V5)]
        [InlineData(XGeometryEngineVersion.V6)]
        public void IfcSphereTest(XGeometryEngineVersion engineVersion )
        {
            using (var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4()))
            {
                using (var txn = m.BeginTransaction("Test"))
                {
                    const double r = 0.5;

                    var sphere = IfcModelBuilder.MakeSphere(m, r);
                    var geomEngine = factory.CreateGeometryEngine(engineVersion, m, _loggerFactory);
                    var solid = geomEngine.CreateSolid(sphere, _logger);
                    solid.Faces.Count.Should().Be(1, "1 face is required of a sphere");
                    solid.Vertices.Count.Should().Be(2, "2 vertices are required of a sphere");
                    var meshRec = new MeshHelper();
                    meshRec.BeginUpdate();
                    geomEngine.Mesh(meshRec, solid, m.ModelFactors.Precision, m.ModelFactors.DeflectionTolerance * 10);
                    meshRec.EndUpdate();
                    meshRec.FaceCount.Should().Be(1, "1 mesh face is required of a sphere");
                    meshRec.PointCount.Should().Be(19, "19 mesh points are required of a sphere");
                    meshRec.TriangleCount.Should().Be(28, "28 triangles are required of a sphere");
                    (meshRec.TriangleCount * 3).Should().Be(meshRec.TriangleIndicesCount, "Incorrect triangulation");
                    txn.Commit();
                }
            }
        }

        
        


    }
}
