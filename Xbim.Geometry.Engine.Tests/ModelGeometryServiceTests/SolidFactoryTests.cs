﻿using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

    public class SolidFactoryTests
    {
        #region Setup

        private readonly ILoggerFactory _loggerFactory;
        private readonly IXbimGeometryServicesFactory _factory;
        readonly IXModelGeometryService _modelSvc;
        public SolidFactoryTests(ILoggerFactory loggerFactory, IXbimGeometryServicesFactory factory)
        {
            _loggerFactory = loggerFactory;
            _factory = factory;
            var dummyModel = new MemoryModel(new EntityFactoryIfc4());
            dummyModel.ModelFactors = new XbimModelFactors(1, 0.001, 1e-5);
            _modelSvc = factory.CreateModelGeometryService(dummyModel, _loggerFactory);
        }

        #endregion

        #region CSG test
        [Fact]
        public void Can_create_csg_block()
        {

            var solidFactory = _modelSvc.SolidFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var solid = solidFactory.Build(blockMoq) as IXSolid; //initialise the factory with the block
            Assert.Equal(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            shells.Should().HaveCount(1);
            Assert.Equal(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            faces.Should().HaveCount(6);
            foreach (var face in faces)
            {
                Assert.NotNull(face.OuterBound);
                Assert.Equal(XShapeType.Wire, face.OuterBound.ShapeType);
                face.InnerBounds.Should().BeEmpty();
                Assert.Equal(XShapeType.Face, face.ShapeType);
                var planarSurface = face.Surface as IXPlane;
                Assert.NotNull(planarSurface);
                Assert.Equal(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                foreach (var edge in face.OuterBound.EdgeLoop)
                {
                    Assert.Equal(XShapeType.Edge, edge.ShapeType);
                    var lineSeg = edge.EdgeGeometry as IXLine;
                    Assert.NotNull(lineSeg);
                }
            }

        }
        [Fact]
        public void Can_create_csg_rectangular_pyramid()
        {

            var solidFactory = _modelSvc.SolidFactory;
            var pyramidMoq = IfcMoq.IfcRectangularPyramidMoq();

            var solid = solidFactory.Build(pyramidMoq) as IXSolid; //initialise the factory with the block
            Assert.Equal(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            shells.Count().Should().Be(1);
            Assert.Equal(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            faces.Count().Should().Be(5);
            foreach (var face in faces)
            {
                Assert.NotNull(face.OuterBound);
                Assert.Equal(XShapeType.Wire, face.OuterBound.ShapeType);
                face.InnerBounds.Should().BeEmpty();
                Assert.Equal(XShapeType.Face, face.ShapeType);
                var planarSurface = face.Surface as IXPlane;
                Assert.NotNull(planarSurface);
                Assert.Equal(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                foreach (var edge in face.OuterBound.EdgeLoop)
                {
                    Assert.Equal(XShapeType.Edge, edge.ShapeType);
                    var lineSeg = edge.EdgeGeometry as IXLine;
                    Assert.NotNull(lineSeg);
                }
            }
        }
        [Fact]
        public void Can_create_csg_cone()
        {

            var rccMoq = IfcMoq.IfcRightCircularConeMoq();

            var solidFactory = _modelSvc.SolidFactory;
            var solid = solidFactory.Build(rccMoq) as IXSolid; //initialise the factory with the block
            Assert.Equal(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            shells.Count().Should().Be(1);
            Assert.Equal(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            faces.Count().Should().Be(2);
            foreach (var face in faces)
            {
                var outerBound = face.OuterBound;
                Assert.NotNull(outerBound);
                Assert.Equal(XShapeType.Wire, outerBound.ShapeType);
                face.InnerBounds.Should().BeEmpty();
                Assert.Equal(XShapeType.Face, face.ShapeType);
                if (face.Surface is IXConicalSurface conicalSurface)
                {
                    //this is the curved surface
                    Assert.Equal(XSurfaceType.IfcSurfaceOfRevolution, conicalSurface.SurfaceType);
                    outerBound.EdgeLoop.Count().Should().Be(4);
                    foreach (var edge in outerBound.EdgeLoop)
                    {
                        Assert.Equal(XShapeType.Edge, edge.ShapeType);
                        var edgeGeom = edge.EdgeGeometry;
                        if (edgeGeom == null) //happens for the top point of the cone, the the start and end points must be the same
                        {
                            var start = edge.EdgeStart.VertexGeometry;
                            var end = edge.EdgeEnd.VertexGeometry;
                            start.X.Should().BeApproximately(end.X, 1e-5);
                            start.Y.Should().BeApproximately(end.Y, 1e-5);
                            start.Z.Should().BeApproximately(end.Z, 1e-5);
                        }
                    }
                }
                else
                {
                    //its the circular plane
                    var planarSurface = face.Surface as IXPlane;
                    Assert.Equal(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                    outerBound.EdgeLoop.Count().Should().Be(1);
                    var circleEdge = outerBound.EdgeLoop.First();
                    Assert.Equal(XCurveType.IfcCircle, circleEdge.EdgeGeometry.CurveType);
                    var circle = circleEdge.EdgeGeometry as IXCircle;
                    Assert.NotNull(circle);
                    Assert.Equal<double>(rccMoq.BottomRadius, circle.Radius);
                }

            }
        }

        [Fact]
        public void Can_create_csg_cylinder()
        {

            var rccMoq = IfcMoq.IfcRightCircularCylinderMoq();

            var solidFactory = _modelSvc.SolidFactory;
            var solid = solidFactory.Build(rccMoq) as IXSolid; //initialise the factory with the block
            Assert.Equal(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            shells.Count().Should().Be(1);
            Assert.Equal(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            Assert.Equal(3, faces.Count());
            Assert.Equal(2, faces.Count(f => f.Surface.SurfaceType == XSurfaceType.IfcPlane)); //2 planar faces
            Assert.Equal(1, faces.Count(f => f.Surface.SurfaceType == XSurfaceType.IfcCylindricalSurface)); //1 cylindrical
            foreach (var face in faces)
            {
                var outerBound = face.OuterBound;
                Assert.NotNull(outerBound);
                Assert.Equal(XShapeType.Wire, outerBound.ShapeType);
                face.InnerBounds.Should().BeEmpty();
                Assert.Equal(XShapeType.Face, face.ShapeType);
                if (face.Surface is IXCylindricalSurface cylindricalSurface) //we expect 2 circles and 2 edges
                {
                    //this is the curved surface
                    Assert.Equal(XSurfaceType.IfcCylindricalSurface, cylindricalSurface.SurfaceType);
                    outerBound.EdgeLoop.Should().HaveCount(4);
                    outerBound.EdgeLoop.Count(e => e.EdgeGeometry.CurveType == XCurveType.IfcCircle).Should().Be(2);
                    outerBound.EdgeLoop.Count(e => e.EdgeGeometry.CurveType == XCurveType.IfcLine).Should().Be(2);
                }
                else
                {
                    //its the circular plane
                    var planarSurface = face.Surface as IXPlane;
                    Assert.Equal(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                    outerBound.EdgeLoop.Should().HaveCount(1);
                    var circleEdge = outerBound.EdgeLoop.First();
                    Assert.Equal(XCurveType.IfcCircle, circleEdge.EdgeGeometry.CurveType);
                    var circle = circleEdge.EdgeGeometry as IXCircle;
                    Assert.NotNull(circle);
                    Assert.Equal<double>(rccMoq.Radius, circle.Radius);
                }
            }
        }

        [Fact]
        public void Can_create_csg_sphere()
        {

            var sphereMoq = IfcMoq.IfcSphereMoq();

            var solidFactory = _modelSvc.SolidFactory;
            var solid = solidFactory.Build(sphereMoq) as IXSolid; //initialise the factory with the block
            Assert.Equal(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            shells.Should().HaveCount(1);
            Assert.Equal(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            faces.Should().HaveCount(1);
            var face = faces.First();
            Assert.NotNull(face.OuterBound);
            Assert.Equal(XShapeType.Wire, face.OuterBound.ShapeType);
            face.InnerBounds.Should().BeEmpty();
            Assert.Equal(XShapeType.Face, face.ShapeType);
            var sphericalSurface = face.Surface as IXSphericalSurface;
            Assert.NotNull(sphericalSurface);
            Assert.Equal(XSurfaceType.IfcSphericalSurface, sphericalSurface.SurfaceType);
            face.OuterBound.EdgeLoop.Should().HaveCount(4);
            face.OuterBound.EdgeLoop.Count(e => e.EdgeGeometry?.CurveType == XCurveType.IfcCircle).Should().Be(2); //two circles
            face.OuterBound.EdgeLoop.Count(e => e.EdgeGeometry == null).Should().Be(2); //2 empty vertex loops
        }
        #endregion

        #region Swept Disks
        [Fact]
        public void Can_create_swept_disk_solid_with_line_directrix()
        {


            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(startParam: 0, endParam: 100, radius: 30, innerRadius: 15);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
            Assert.False(solid.IsEmptyShape());
            Assert.True(solid.IsValidShape());
            double volume = solid.Volume;
            volume.Should().BeGreaterThan(0);
        }
        [Fact]
        public void Can_create_swept_disk_solid_with_polyline_directrix()
        {

            var solidService = _modelSvc.SolidFactory;
            var points = new (double X, double Y, double Z)[] { (X: 0, Y: 0, Z: 0), (X: 1000, Y: 0, Z: 0), (X: 1000, Y: 1500, Z: 0), (X: 0, Y: 1500, Z: 0) };
            var pline = IfcMoq.IfcPolylineMock(dim: 3);
            foreach (var (X, Y, Z) in points)
            {
                pline.Points.Add(IfcMoq.IfcCartesianPoint3dMock(X, Y, Z));
            }
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: pline, startParam: 0, endParam: null, radius: 30, innerRadius: 15);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
            var s = solid.BrepString();
            Assert.False(solid.IsEmptyShape());
            Assert.True(solid.IsValidShape());
            double volume = solid.Volume;
            volume.Should().BeGreaterThan(0);
        }
        [Fact]

        public void Can_create_swept_disk_solid_with_composite_curve_directrix()
        {
            var solidService = _modelSvc.SolidFactory;
            var directrix = IfcMoq.TypicalCompositeCurveMock(_modelSvc.CurveFactory, out double totalParametricLength, out double totalLength);
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: directrix, radius: 10, innerRadius: 8);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
            Assert.False(solid.IsEmptyShape());
            Assert.True(solid.IsValidShape());
            double volume = solid.Volume;
            volume.Should().BeApproximately(14052.092819053487, _modelSvc.Precision);
        }
        [Theory]
        [InlineData(30, 20, 0, Math.PI, 100)]
        [InlineData(30, 20, Math.PI, 3 * Math.PI / 2, 100)]
        public void Can_create_swept_disk_solid_with_trimmed_circle_directrix(double outerRadius, double innerRadius, double startParam, double endParam, double directrixRadius)
        {
            //NB Periodic directixes (circles ellipses) must be specified in a parametric space of radians
            var solidService = _modelSvc.SolidFactory;
            var circle = IfcMoq.IfcCircle3dMock(radius: 100);
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: circle, startParam: startParam, endParam: endParam, radius: outerRadius, innerRadius: innerRadius);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
#if DEBUG
            var bstr = solid.BrepString();
#endif
            Assert.False(solid.IsEmptyShape());
            Assert.True(solid.IsValidShape());
            double volume = solid.Volume;
            double outerArea = Math.PI * outerRadius * outerRadius;
            double innerArea = Math.PI * innerRadius * innerRadius;
            double directixLen = directrixRadius * (endParam - startParam);
            double desiredVolume = (outerArea - innerArea) * directixLen;
            volume.Should().BeApproximately(desiredVolume, 1e-5);
        }

        [Theory]
        [InlineData(30, 20, 0, Math.PI, 100, 200, 760929)]
        public void Can_create_swept_disk_solid_with_trimmed_elipse_directrix(double outerRadius, double innerRadius, double startParam, double endParam, double semi1, double semi2, double calcVolume)
        {
            //NB Periodic directixes (circles ellipses) must be specified in a parametric space of radians
            var solidService = _modelSvc.SolidFactory;
            var ellipse = IfcMoq.IfcEllipse3dMock(semi1: semi1, semi2: semi2);
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: ellipse, startParam: startParam, endParam: endParam, radius: outerRadius, innerRadius: innerRadius);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
#if DEBUG
            var bstr = solid.BrepString();
#endif
            Assert.False(solid.IsEmptyShape());
            Assert.True(solid.IsValidShape());

            solid.Volume.Should().BeApproximately(calcVolume, 1);
        }
        #endregion

        #region Swept Areas
        [Fact]
        public void Can_create_swept_area_solid_with_rectangle_profile_def()
        {

            var solidService = _modelSvc.SolidFactory;
            var rectProfileDef = IfcMoq.IfcRectangleProfileDefMock(x: 200, y: 400);
            var extrudedArea = IfcMoq.IfcSweptAreaSolidMoq(sweptArea: rectProfileDef, depth: 300);
            var extrusion = (IXSolid)solidService.Build(extrudedArea);
            extrusion.Volume.Should().Be(200 * 400 * 300);
        }

        [Fact]
        public void Can_create_swept_area_solid_with_circle_profile_def()
        {

            var solidService = _modelSvc.SolidFactory;
            var circleProfileDef = IfcMoq.IfcCircleProfileDefMock(radius: 200);
            var extrudedArea = IfcMoq.IfcSweptAreaSolidMoq(sweptArea: circleProfileDef, depth: 900);
            var extrusion = (IXSolid)solidService.Build(extrudedArea);
            extrusion.Volume.Should().Be((Math.PI * 200 * 200) * 900);
        }

        [Fact]
        public void Can_extrude_arbitrary_profile_def_with_voids()
        {
            using var model = MemoryModel.OpenRead("testfiles/ExtrudedAreaSolidFailsOnExtrusion.ifc");
            var engine = _factory.CreateGeometryEngineV6(model, _loggerFactory);
            var ifcExtrudedAreaSolid = model.Instances[1] as IIfcExtrudedAreaSolid;
            var v6Solid = engine.SolidFactory.Build(ifcExtrudedAreaSolid) as IXSolid;
            Assert.NotNull(v6Solid);
            var v5Solid = engine.CreateSolid(ifcExtrudedAreaSolid);
            Assert.NotNull(v5Solid);
            v5Solid.Volume.Should().BeApproximately(v6Solid.Volume, 1e-5);
        }
        #endregion
    }

}

