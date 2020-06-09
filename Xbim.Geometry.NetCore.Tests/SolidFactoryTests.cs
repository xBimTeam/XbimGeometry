using Extensions.Logging.ListOfString;
using FluentAssertions;
using FluentAssertions.Common;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Services;

namespace Xbim.Geometry.NetCore.Tests
{
    [TestClass]
    public class SolidFactoryTests
    {
        #region Setup


        static IHost serviceHost;
        static IServiceScope _modelScope;
        [ClassInitialize]
        static public async System.Threading.Tasks.Task InitialiseAsync(TestContext context)
        {
            serviceHost = CreateHostBuilder().Build();
            await serviceHost.StartAsync();
            _modelScope = serviceHost.Services.CreateScope();
        }

        public static IHostBuilder CreateHostBuilder() =>
            Host.CreateDefaultBuilder()
                .ConfigureServices((hostContext, services) =>
                {
                    services.AddHostedService<GeometryServicesHost>()
                    .AddSingleton<IXLoggingService, LoggingService>()
                    .AddSingleton<IXShapeService, ShapeService>()
                    .AddScoped<IXSolidService, SolidService>()
                    .AddScoped<IXCurveService, CurveService>()
                    .AddScoped<IXModelService, ModelService>(sp =>
                        new ModelService(IfcMoq.IfcModelMock(millimetre: 1, precision: 1e-5, radianFactor: 1), minGapSize: 1.0));
                })
            .ConfigureLogging((hostContext, loggingBuilder) =>
            {
                loggingBuilder.AddProvider(new StringListLoggerProvider(new StringListLogger(new List<string>(), name: "LoggingService")));
            });

        private IXLoggingService LoggingService
        {
            get => serviceHost.Services.GetService<IXLoggingService>();
        }
        private ILogger Logger
        {
            get => serviceHost.Services.GetService<ILogger<LoggingService>>() as ILogger;
        }
        [ClassCleanup]
        static public async System.Threading.Tasks.Task CleanupAsync()
        {
            await serviceHost.StopAsync();
        }
        #endregion

        #region CSG test
        [TestMethod]
        public void Can_create_csg_block()
        {

            var solidFactory = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var blockMoq = IfcMoq.IfcBlockMoq();
            var solid = solidFactory.Build(blockMoq) as IXSolid; //initialise the factory with the block
            Assert.AreEqual(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            Assert.AreEqual(1, shells.Count());
            Assert.AreEqual(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            Assert.AreEqual(6, faces.Count());
            foreach (var face in faces)
            {
                Assert.IsNotNull(face.OuterBound);
                Assert.AreEqual(XShapeType.Wire, face.OuterBound.ShapeType);
                Assert.AreEqual(0, face.InnerBounds.Count());
                Assert.AreEqual(XShapeType.Face, face.ShapeType);
                var planarSurface = face.Surface as IXPlane;
                Assert.IsNotNull(planarSurface);
                Assert.AreEqual(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                foreach (var edge in face.OuterBound.EdgeLoop)
                {
                    Assert.AreEqual(XShapeType.Edge, edge.ShapeType);
                    var lineSeg = edge.EdgeGeometry as IXLine;
                    Assert.IsNotNull(lineSeg);
                }
            }

        }
        [TestMethod]
        public void Can_create_csg_rectangular_pyramid()
        {
            var solidFactory = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var pyramidMoq = IfcMoq.IfcRectangularPyramidMoq();

            var solid = solidFactory.Build(pyramidMoq) as IXSolid; //initialise the factory with the block
            Assert.AreEqual(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            Assert.AreEqual(1, shells.Count());
            Assert.AreEqual(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            Assert.AreEqual(5, faces.Count());
            foreach (var face in faces)
            {
                Assert.IsNotNull(face.OuterBound);
                Assert.AreEqual(XShapeType.Wire, face.OuterBound.ShapeType);
                Assert.AreEqual(0, face.InnerBounds.Count());
                Assert.AreEqual(XShapeType.Face, face.ShapeType);
                var planarSurface = face.Surface as IXPlane;
                Assert.IsNotNull(planarSurface);
                Assert.AreEqual(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                foreach (var edge in face.OuterBound.EdgeLoop)
                {
                    Assert.AreEqual(XShapeType.Edge, edge.ShapeType);
                    var lineSeg = edge.EdgeGeometry as IXLine;
                    Assert.IsNotNull(lineSeg);
                }
            }

        }
        [TestMethod]
        public void Can_create_csg_cone()
        {
            var rccMoq = IfcMoq.IfcRightCircularConeMoq();
            var solidFactory = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var solid = solidFactory.Build(rccMoq) as IXSolid; //initialise the factory with the block
            Assert.AreEqual(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            Assert.AreEqual(1, shells.Count());
            Assert.AreEqual(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            Assert.AreEqual(2, faces.Count());
            foreach (var face in faces)
            {
                var outerBound = face.OuterBound;
                Assert.IsNotNull(outerBound);
                Assert.AreEqual(XShapeType.Wire, outerBound.ShapeType);
                Assert.AreEqual(0, face.InnerBounds.Count());
                Assert.AreEqual(XShapeType.Face, face.ShapeType);
                var conicalSurface = face.Surface as IXConicalSurface;
                if (conicalSurface != null)
                {
                    //this is the curved surface
                    Assert.AreEqual(XSurfaceType.IfcSurfaceOfRevolution, conicalSurface.SurfaceType);
                    Assert.AreEqual(4, outerBound.EdgeLoop.Count());
                    foreach (var edge in outerBound.EdgeLoop)
                    {
                        Assert.AreEqual(XShapeType.Edge, edge.ShapeType);
                        var edgeGeom = edge.EdgeGeometry;
                        if (edgeGeom == null) //happens for the top point of the cone, the the start and end points must be the same
                        {
                            var start = edge.EdgeStart.VertexGeometry;
                            var end = edge.EdgeEnd.VertexGeometry;
                            Assert.AreEqual(start.X, end.X, 1e-5);
                            Assert.AreEqual(start.Y, end.Y, 1e-5);
                            Assert.AreEqual(start.Z, end.Z, 1e-5);
                        }
                    }
                }
                else
                {
                    //its the circular plane
                    var planarSurface = face.Surface as IXPlane;
                    Assert.AreEqual(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                    Assert.AreEqual(1, outerBound.EdgeLoop.Count());
                    var circleEdge = outerBound.EdgeLoop.First();
                    Assert.AreEqual(XCurveType.IfcCircle, circleEdge.EdgeGeometry.CurveType);
                    var circle = circleEdge.EdgeGeometry as IXCircle;
                    Assert.IsNotNull(circle);
                    Assert.AreEqual<double>(rccMoq.BottomRadius, circle.Radius);
                }

            }
        }

        [TestMethod]
        public void Can_create_csg_cylinder()
        {
            var rccMoq = IfcMoq.IfcRightCircularCylinderMoq();
            var solidFactory = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var solid = solidFactory.Build(rccMoq) as IXSolid; //initialise the factory with the block
            Assert.AreEqual(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            Assert.AreEqual(1, shells.Count());
            Assert.AreEqual(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            Assert.AreEqual(3, faces.Count());
            Assert.AreEqual(2, faces.Count(f => f.Surface.SurfaceType == XSurfaceType.IfcPlane)); //2 planar faces
            Assert.AreEqual(1, faces.Count(f => f.Surface.SurfaceType == XSurfaceType.IfcCylindricalSurface)); //1 cylindrical
            foreach (var face in faces)
            {
                var outerBound = face.OuterBound;
                Assert.IsNotNull(outerBound);
                Assert.AreEqual(XShapeType.Wire, outerBound.ShapeType);
                Assert.AreEqual(0, face.InnerBounds.Count());
                Assert.AreEqual(XShapeType.Face, face.ShapeType);
                var cylindricalSurface = face.Surface as IXCylindricalSurface;
                if (cylindricalSurface != null) //we expect 2 circles and 2 edges
                {
                    //this is the curved surface
                    Assert.AreEqual(XSurfaceType.IfcCylindricalSurface, cylindricalSurface.SurfaceType);
                    Assert.AreEqual(4, outerBound.EdgeLoop.Count());
                    Assert.AreEqual(2, outerBound.EdgeLoop.Count(e => e.EdgeGeometry.CurveType == XCurveType.IfcCircle));
                    Assert.AreEqual(2, outerBound.EdgeLoop.Count(e => e.EdgeGeometry.CurveType == XCurveType.IfcLine));
                }
                else
                {
                    //its the circular plane
                    var planarSurface = face.Surface as IXPlane;
                    Assert.AreEqual(XSurfaceType.IfcPlane, planarSurface.SurfaceType);
                    Assert.AreEqual(1, outerBound.EdgeLoop.Count());
                    var circleEdge = outerBound.EdgeLoop.First();
                    Assert.AreEqual(XCurveType.IfcCircle, circleEdge.EdgeGeometry.CurveType);
                    var circle = circleEdge.EdgeGeometry as IXCircle;
                    Assert.IsNotNull(circle);
                    Assert.AreEqual<double>(rccMoq.Radius, circle.Radius);
                }
            }
        }

        [TestMethod]
        public void Can_create_csg_sphere()
        {
            var sphereMoq = IfcMoq.IfcSphereMoq();

            var solidFactory = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var solid = solidFactory.Build(sphereMoq) as IXSolid; //initialise the factory with the block
            Assert.AreEqual(XShapeType.Solid, solid.ShapeType);
            var shells = solid.Shells;
            Assert.AreEqual(1, shells.Count());
            Assert.AreEqual(XShapeType.Shell, shells.First().ShapeType);
            var faces = shells.First().Faces;
            Assert.AreEqual(1, faces.Count());
            var face = faces.First();
            Assert.IsNotNull(face.OuterBound);
            Assert.AreEqual(XShapeType.Wire, face.OuterBound.ShapeType);
            Assert.AreEqual(0, face.InnerBounds.Count());
            Assert.AreEqual(XShapeType.Face, face.ShapeType);
            var sphericalSurface = face.Surface as IXSphericalSurface;
            Assert.IsNotNull(sphericalSurface);
            Assert.AreEqual(XSurfaceType.IfcSphericalSurface, sphericalSurface.SurfaceType);
            Assert.AreEqual(4, face.OuterBound.EdgeLoop.Count());
            Assert.AreEqual(2, face.OuterBound.EdgeLoop.Count(e => e.EdgeGeometry?.CurveType == XCurveType.IfcCircle)); //two circles
            Assert.AreEqual(2, face.OuterBound.EdgeLoop.Count(e => e.EdgeGeometry == null)); //2 empty vertex loops


        }
        #endregion

        #region Swept Disks
        [TestMethod]
        public void Can_create_swept_disk_solid_with_line_directrix()
        {
            var solidService = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(startParam:0, endParam: 100, radius:30, innerRadius: 15);
            var solid = solidService.Build(ifcSweptDisk);
            Assert.IsFalse(solid.IsEmptyShape());
            Assert.IsTrue(solid.IsValidShape());
            double volume = solid.Volume();
            volume.Should().BeGreaterThan(0);
        }
        [TestMethod]
        public void Can_create_swept_disk_solid_with_polyline_directrix()
        {
            var solidService = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var points = new (double X, double Y, double Z)[] { (X: 0, Y: 0, Z:0), (X: 1000, Y: 0, Z: 0), (X: 1000, Y: 1500, Z: 0), (X: 0, Y: 1500, Z: 0) };
            var pline = IfcMoq.IfcPolylineMock(dim: 3);
            foreach (var p in points)
            {
                pline.Points.Add(IfcMoq.IfcCartesianPoint3dMock(p.X, p.Y, p.Z));
            }
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: pline,  startParam: 0, endParam: -1, radius: 30, innerRadius: 15);
            var solid = solidService.Build(ifcSweptDisk);
            Assert.IsFalse(solid.IsEmptyShape());
            Assert.IsTrue(solid.IsValidShape());
            double volume = solid.Volume();
            volume.Should().BeGreaterThan(0);
        }
        [TestMethod]
        public void Can_create_swept_disk_solid_with_composite_curve_directrix()
        {
            var solidService = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var curveService = _modelScope.ServiceProvider.GetRequiredService<IXCurveService>();
            var modelService = _modelScope.ServiceProvider.GetRequiredService<IXModelService>();
            double totalLength, totalParametricLength;
            var directrix = IfcMoq.TypicalCompositeCurveMock(curveService,out totalParametricLength, out totalLength);

            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: directrix, radius: 10, innerRadius: 8);
            var solid = solidService.Build(ifcSweptDisk);
            Assert.IsFalse(solid.IsEmptyShape());
            Assert.IsTrue(solid.IsValidShape());
            double volume = solid.Volume();
            volume.Should().BeApproximately(14052.09281905348, modelService.Precision);
        }
        [TestMethod]
        public void Can_create_swept_disk_solid_with_trimmed_circle_directrix()
        {
            var solidService = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var circle = IfcMoq.IfcCircle3dMock(radius: 100);
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: circle, startParam: Math.PI, endParam: 0 , radius: 30, innerRadius: 15);
            var solid = solidService.Build(ifcSweptDisk);
            Assert.IsFalse(solid.IsEmptyShape());
            Assert.IsTrue(solid.IsValidShape());
            double volume = solid.Volume();
            volume.Should().BeGreaterThan(0);
        }

        [TestMethod]
        public void Can_create_swept_disk_solid_with_trimmed_elipse_directrix()
        {
            var solidService = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var ellipse = IfcMoq.IfcEllipse3dMock(semi1: 200, semi2:300);
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq(directrix: ellipse, startParam: Math.PI/2, endParam: 0, radius: 30, innerRadius: 15);
            var solid = solidService.Build(ifcSweptDisk);
            Assert.IsFalse(solid.IsEmptyShape());
            Assert.IsTrue(solid.IsValidShape());
            double volume = solid.Volume();
            volume.IsSameOrEqualTo(2523289.1559284292);
            
        }
        #endregion

        #region Swept Areas
        [TestMethod]
        public void Can_create_swept_area_solid_with_rectangle_profile_def()
        {
            var solidService = _modelScope.ServiceProvider.GetRequiredService<IXSolidService>();
            var rectProfileDef = IfcMoq.IfcRectangleProfileDefMock(x: 200, y: 400);
            var extrudedArea = IfcMoq.IfcSweptAreaSolidMoq(sweptArea: rectProfileDef, depth:300);
            var extrusion = solidService.Build(extrudedArea);
            extrusion.Volume().Should().Be(200*400*300);
        }
        #endregion

    }
}
