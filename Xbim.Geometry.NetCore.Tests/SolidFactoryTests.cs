using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Xbim.Common;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Factories;
using Xbim.Geometry.Services;
using Xbim.IO.Memory;

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
                    .AddScoped<IXSolidService, SolidFactory>()
                    .AddScoped<IXModelService, ModelService>(sp => 
                        new ModelService(IfcMoq.IfcModelMock(millimetre: 1, precision: 1e-5, radianFactor: 1),minGapSize: 1.0));
                })
            .ConfigureLogging((hostContext, loggingBuilder) =>
            {
                loggingBuilder.AddConsole((config) => config.IncludeScopes = true).AddDebug();
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
    }
}
