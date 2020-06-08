using Extensions.Logging.ListOfString;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Factories;
using Xbim.Geometry.Services;
using System.Linq;
using System.Threading.Tasks;
using System.Diagnostics;
using FluentAssertions.Common;
using FluentAssertions;

namespace Xbim.Geometry.NetCore.Tests
{
    [TestClass]
    public class WireFactoryTests
    {
        #region Class Setup
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
                .AddScoped<IXWireService, WireService>()
                .AddScoped<IXCurveService, CurveService>()
                .AddScoped<IXModelService, ModelService>(sp =>
                        new ModelService(IfcMoq.IfcModelMock(millimetre: 1, precision: 1e-5, radianFactor: 1), minGapSize: 1.0));
            })
        .ConfigureLogging((hostContext, loggingBuilder) =>
        {
            loggingBuilder.AddProvider(new StringListLoggerProvider(new StringListLogger(new List<string>(), name: "LoggingService")));
        });


        [ClassCleanup]
        static public async System.Threading.Tasks.Task CleanupAsync()
        {

            await serviceHost.StopAsync();
        }
        #endregion
        public struct PolyData
        {
            public string Name;
            public int EdgeCount;
            public (double X, double Y)[] Points;
        }

        public static List<PolyData> Polyline2dData = new List<PolyData>()
        {
          new PolyData{Name="NoDuplicatePoints" ,EdgeCount=4, Points=new (double X, double Y) [] { (X: 0, Y: 0), (X: 10, Y: 0), (X: 10, Y: 15), (X: 0, Y: 15), (X: 0, Y: 0) }},
          new PolyData{Name="OneDuplicatePointsInTolerance",EdgeCount=4, Points= new (double X, double Y) [] { (X: 0, Y: 0), (X: 10, Y: 0), (X: 10.00001, Y: 0), (X: 10, Y: 15), (X: 0, Y: 15), (X: 0, Y: 0) } },
          new PolyData{Name="TwoDuplicatePointsInTolerance",EdgeCount=4, Points= new (double X, double Y) [] { (X: 0, Y: 0), (X: 10, Y: 0), (X: 10.00001, Y: 0), (X: 9.99999, Y: 0), (X: 10, Y: 15), (X: 0, Y: 15), (X: 0, Y: 0) } },
          new PolyData{Name="TwoDuplicateEndPointsInTolerance",EdgeCount=4, Points= new (double X, double Y) [] { (X: 0, Y: 0), (X: 10, Y: 0), (X: 10, Y: 15), (X: 0, Y: 15), (X: 0, Y: 0.000009), (X: 0, Y: 0) } },
          new PolyData{Name="TwoDuplicateStartPointsInTolerance",EdgeCount=4, Points= new (double X, double Y) [] { (X: 0, Y: 0.000009),(X: 0, Y: 0), (X: 10, Y: 0), (X: 10, Y: 15), (X: 0, Y: 15),  (X: 0, Y: 0) } },
          new PolyData{Name= "SelfInterectingLine",EdgeCount=5, Points= new(double X, double Y)[] { (X: 0, Y: 0), (X: 10, Y: 0), (X: 5, Y: 0), (X: 10, Y: 0), (X: 10, Y: 15), (X: 0, Y: 15), (X: 0, Y: 0) }},
          new PolyData{Name="IntersectingLines",EdgeCount=6, Points=  new (double X, double Y) [] { (X: 0, Y: 0), (X: 10, Y: 0), (X: 0, Y: 15), (X: 10, Y: 15), (X: 0, Y: 0) } }
        };
        public static IEnumerable<object[]> Polyline2dDataSource
        {
            get
            {
                foreach (var data in Polyline2dData)
                {
                    yield return new object[] { data.Name, data.EdgeCount, data.Points };
                }
            }
        }
        [DataTestMethod]
        [DynamicData(nameof(Polyline2dDataSource), DynamicDataSourceType.Property)]
        public void Can_build_polyline2d(string dataSetName, int edgeCount, (double X, double Y)[] points)
        {
            var polyline = IfcMoq.IfcPolylineMock(dim: 2);
            int ifcLabel = 100;
            foreach (var point in points)
            {
                polyline.Points.Add(IfcMoq.IfcCartesianPoint2dMock(point.X, point.Y, ifcLabel++));
            }
            //get the profile service
            var wireService = _modelScope.ServiceProvider.GetRequiredService<IXWireService>();
            var wire = wireService.Build(polyline);
            Assert.AreEqual(edgeCount, wire.EdgeLoop.Count());
        }

        [DataTestMethod]
        [DynamicData(nameof(Polyline2dDataSource), DynamicDataSourceType.Property)]
        public async Task Can_build_polyline2dAsync(string dataSetName, int edgeCount, (double X, double Y)[] points)
        {
            //do 10 at a time
            var polyline = IfcMoq.IfcPolylineMock(dim: 2);
            int ifcLabel = 100;
            foreach (var point in points)
            {
                polyline.Points.Add(IfcMoq.IfcCartesianPoint2dMock(point.X, point.Y, ifcLabel++));
            }
            //get the profile service

            var wireService = _modelScope.ServiceProvider.GetRequiredService<IXWireService>();

            var sw = new Stopwatch();
            sw.Start();
            var result = new List<IXWire>(10);
            //do it normally
            for (int i = 0; i < 10; i++)
            {
                result.Add(wireService.Build(polyline));
            }

            sw.Stop();
            var nonAsyncTime = sw.ElapsedMilliseconds;
            sw.Restart();

            var taskResults = new List<Task<IXWire>>(10);
            for (int i = 0; i < 10; i++)
            {
                taskResults.Add(wireService.BuildAsync(polyline));
            }
            await Task.WhenAll(taskResults).ConfigureAwait(false);

            sw.Stop();
            var asyncTime = sw.ElapsedMilliseconds;
            nonAsyncTime.Should().BeGreaterThan(asyncTime);
           
            foreach (var taskResult in taskResults)
            {
                Assert.IsTrue(taskResult.IsCompletedSuccessfully);
                Assert.AreEqual(edgeCount, taskResult.Result.EdgeLoop.Count());
            }
        }

        [DataTestMethod]
        [DynamicData(nameof(Polyline2dDataSource), DynamicDataSourceType.Property)]
        public void Can_build_polyline(string dataSetName, int edgeCount, (double X, double Y)[] points)
        {
            if (dataSetName == "SelfInterectingLine" || dataSetName == "IntersectingLines") return; //skip this test as it cannot be validated
            var polyline = IfcMoq.IfcPolylineMock(dim: 3);
            int ifcLabel = 100;
            foreach (var point in points)
            {
                polyline.Points.Add(IfcMoq.IfcCartesianPoint3dMock(point.X, point.Y, 0, ifcLabel++));
            }
            //get the profile service
            var wireService = _modelScope.ServiceProvider.GetRequiredService<IXWireService>();
            var wire = wireService.Build(polyline);
            Assert.AreEqual(edgeCount, wire.EdgeLoop.Count());
        }

        [TestMethod]
        public void Can_build_composite_curve_wire()
        {
            var wireService = _modelScope.ServiceProvider.GetRequiredService<IXWireService>();
            var curveService = _modelScope.ServiceProvider.GetRequiredService<IXCurveService>();
            var modelService = _modelScope.ServiceProvider.GetRequiredService<IXModelService>();
            double totalParametricLength, totalLength;
            var ifcCompCurve = IfcMoq.TypicalCompositeCurveMock(curveService, out totalParametricLength, out totalLength);
            var wire = wireService.Build(ifcCompCurve);
            Assert.AreEqual(XShapeType.Wire, wire.ShapeType);
            wire.Length().Should().BeApproximately(totalLength, modelService.Precision);

        }

    }
}
