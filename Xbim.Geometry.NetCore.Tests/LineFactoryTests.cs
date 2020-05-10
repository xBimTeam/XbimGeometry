
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Extensions.Logging;
using System;
using System.Diagnostics;

using Xbim.Geometry.Services;
using Xbim.Common.Geometry;
using Xbim.Geometry.Exceptions;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xbim.Common;
using Xbim.Ifc2x3.GeometryResource;
using Xbim.Ifc4.Interfaces;
using Xbim.Geometry.Factories;
using Xbim.Geometry.Abstractions;
using Moq;
using Xbim.Ifc4.MeasureResource;

namespace Xbim.Geometry.NetCore.Tests
{
    [TestClass]
    public class LineFactoryTests
    {
        static IHost serviceHost;
        static IModel model;

        [ClassInitialize]
        static public async System.Threading.Tasks.Task InitialiseAsync(TestContext context)
        {
            model = new MemoryModel(new EntityFactoryIfc4());
            serviceHost = CreateHostBuilder().Build();
            await serviceHost.StartAsync();


        }
        public static IHostBuilder CreateHostBuilder() =>
        Host.CreateDefaultBuilder()
            .ConfigureServices((hostContext, services) =>
            {
                services.AddHostedService<LoggingService>();
            })
        .ConfigureLogging((hostContext, loggingBuilder) =>
        {
            loggingBuilder.AddConsole((config) => config.IncludeScopes = true).AddDebug();
        });

        private LoggingService LoggingService
        {
            get => serviceHost.Services.GetService<IHostedService>() as LoggingService;
        }

        [ClassCleanup]
        static public async System.Threading.Tasks.Task CleanupAsync()
        {
            model = null;
            await serviceHost.StopAsync();
        }

        [TestMethod]
        public void Can_raise_exception_with_bad_direction()
        {

            //using (var lineFactory = new LineFactory(LoggingService))
            //{
            //    //illegal vector
            //    Assert.ThrowsException<XbimGeometryFactoryException>(() => lineFactory.Direction = new XbimVector3D(0, 0, 0));
            //}
        }

        [TestMethod]
        public void Can_convert_ifc_line_3d()
        {
            var ifcLine = IfcMoq.IfcLine3dMock();
            using (var lineFactory = new CurveFactory(LoggingService))
            {
                var line = lineFactory.Build3d(ifcLine); //initialise the factory with the line
                Assert.AreEqual(ifcLine.Pnt.X, line.Origin.X);
                Assert.AreEqual(ifcLine.Pnt.Y, line.Origin.Y);
                Assert.AreEqual(ifcLine.Pnt.Z, line.Origin.Z);
                Assert.AreEqual<double>(ifcLine.Dir.Magnitude, line.Direction.Magnitude);
                Assert.AreEqual(ifcLine.Dir.Orientation.X, line.Direction.X);
                Assert.AreEqual(ifcLine.Dir.Orientation.Y, line.Direction.Y);
                Assert.AreEqual(ifcLine.Dir.Orientation.Z, line.Direction.Z);
                var p1 = line.GetPoint(500);
                Assert.AreEqual((500 * ifcLine.Dir.Magnitude) + ifcLine.Pnt.Z, p1.Z); //The parametric unit (magnitude) is 10, so 500 * 10 is the distance
                var p2 = line.GetFirstDerivative(500, out IXVector normal);
                Assert.AreEqual(p1.X, p2.X);
                Assert.AreEqual(p1.Y, p2.Y);
                Assert.AreEqual(p1.Z, p2.Z);
                Assert.AreEqual(line.Direction.X, normal.X);
                Assert.AreEqual(line.Direction.Y, normal.Y);
                Assert.AreEqual(line.Direction.Z, normal.Z);
                Assert.IsTrue(line.Is3d);
            }

        }
        [TestMethod]
        public void Can_convert_ifc_line_2d()
        {

            var ifcLine = IfcMoq.IfcLine2dMock();
            using (var lineFactory = new CurveFactory(LoggingService))
            {
                var line = lineFactory.Build2d(ifcLine); //initialise the factory with the line
                Assert.AreEqual(ifcLine.Pnt.X, line.Origin.X);
                Assert.AreEqual(ifcLine.Pnt.Y, line.Origin.Y);
                Assert.AreEqual<double>(ifcLine.Dir.Magnitude, line.Direction.Magnitude);
                Assert.AreEqual(ifcLine.Dir.Orientation.X, line.Direction.X);
                Assert.AreEqual(ifcLine.Dir.Orientation.Y, line.Direction.Y);

                var p1 = line.GetPoint(500);
                Assert.AreEqual((500 * ifcLine.Dir.Magnitude) + ifcLine.Pnt.Y, p1.Y); //The parametric unit (magnitude) is 10, so 500 * 10 is the distance
                var p2 = line.GetFirstDerivative(500, out IXVector normal);
                Assert.AreEqual(p1.X, p2.X);
                Assert.AreEqual(p1.Y, p2.Y);

                Assert.AreEqual(line.Direction.X, normal.X);
                Assert.AreEqual(line.Direction.Y, normal.Y);

                Assert.IsFalse(line.Is3d);
                Assert.ThrowsException<XbimGeometryFactoryException>(() => p1.Z);
                Assert.ThrowsException<XbimGeometryFactoryException>(() => normal.Z);
            }

        }
        [TestMethod]
        public void Cannot_build_2D_line_from_3D_IfcLine()
        {
            var lineMoq = IfcMoq.IfcLine3dMock();
            using (var lineFactory = new CurveFactory(LoggingService))
            {
                var line = lineFactory.Build3d(lineMoq); //should be fine
                Assert.ThrowsException<XbimGeometryFactoryException>(() => lineFactory.Build2d(lineMoq));
            }

        }
        //[TestMethod]
        //public void Can_build_vertex_from_ifc_cartesian_point()
        //{

        //    var logger = testService.Services.GetService<ILogger<TestService>>();
        //    using (var memoryModel = new MemoryModel(new EntityFactoryIfc4()))
        //    {
        //        using (var vertexFactory = new VertexFactory(memoryModel, logger))
        //        {
        //            using (var txn = memoryModel.BeginTransaction("Create point"))
        //            {

        //                var creator = new Create(memoryModel);
        //                var ifcPoint = creator.CartesianPoint((p) =>
        //                {
        //                    p.Coordinates.Add(1);
        //                    p.Coordinates.Add(2);
        //                    p.Coordinates.Add(3);
        //                });
        //                using (var entityScope = logger.BeginScope("Id: #{ifc_id}={ifc_type}", ifcPoint.EntityLabel, ifcPoint.GetType().Name))
        //                {
        //                    var vertex = vertexFactory.Create(1, 2, 3);
        //                    Assert.AreEqual(ifcPoint.X, vertex.GeomProps.X);
        //                    Assert.AreEqual(ifcPoint.Y, vertex.GeomProps.Y);
        //                    Assert.AreEqual(ifcPoint.Z, vertex.GeomProps.Z);
        //                    Assert.AreEqual(memoryModel.ModelFactors.Precision, vertex.GeomProps.Tolerance, 1e-9);

        //                }
        //            }
        //        }
        //    }

        //}
    }
}
