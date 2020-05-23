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
                .AddScoped<IXWireService, WireFactory>()
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
        struct PolyData
        {
            public string Name;
            public int EdgeCount;
            public (double X, double Y)[] Points;
        }

        static List<PolyData> Polyline2dData = new List<PolyData>()
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
            var polyline = IfcMoq.IfcPolyline2dMock();
            int ifcLabel = 100;
            foreach (var point in points)
            {
                polyline.Points.Add(IfcMoq.IfcCartesianPoint2dMock(point.X, point.Y, ifcLabel++));
            }
            //get the profile service
            var wireFactory = _modelScope.ServiceProvider.GetRequiredService<IXWireService>();
            var wire = wireFactory.Build(polyline);
            Assert.AreEqual(edgeCount, wire.EdgeLoop.Count());
        }
    }
}
