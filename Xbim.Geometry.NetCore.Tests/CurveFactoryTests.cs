
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
    public class CurveFactoryTests
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
        private ILogger Logger
        {
            get => serviceHost.Services.GetService<ILogger<LoggingService>>() as ILogger;
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
        #region Line Tests

        [DataTestMethod]
        [DataRow(1)]
        [DataRow(10)]
        [DataRow(-10)]
        [DataRow(0.1)]
        public void Can_convert_ifc_line_3d(double parametricLength)
        {
            var ifcLine = IfcMoq.IfcLine3dMock(magnitude: parametricLength);
            using (var lineFactory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var line = lineFactory.Build(ifcLine) as IXLine; //initialise the factory with the line
                Assert.IsNotNull(line);
                Assert.AreEqual(ifcLine.Pnt.X, line.Origin.X);
                Assert.AreEqual(ifcLine.Pnt.Y, line.Origin.Y);
                Assert.AreEqual(ifcLine.Pnt.Z, line.Origin.Z);
                Assert.AreEqual<double>(ifcLine.Dir.Magnitude, line.Direction.Magnitude);
                Assert.AreEqual(ifcLine.Dir.Orientation.X, line.Direction.X);
                Assert.AreEqual(ifcLine.Dir.Orientation.Y, line.Direction.Y);
                Assert.AreEqual(ifcLine.Dir.Orientation.Z, line.Direction.Z);
                var p1 = line.GetPoint(500);
                Assert.AreEqual((500 * ifcLine.Dir.Magnitude) + ifcLine.Pnt.Z, p1.Z); //The parametric unit (magnitude) is parametricLength, so 500 * parametricLength is the distance
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

        [DataTestMethod]
        [DataRow(1)]
        [DataRow(10)]
        [DataRow(-10)]
        [DataRow(0.1)]
        public void Can_convert_ifc_line_2d(double parametricLength)
        {

            var ifcLine = IfcMoq.IfcLine2dMock(magnitude: parametricLength);
            using (var lineFactory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var line = lineFactory.Build(ifcLine) as IXLine; //initialise the factory with the line
                Assert.IsNotNull(line);
                Assert.AreEqual(ifcLine.Pnt.X, line.Origin.X);
                Assert.AreEqual(ifcLine.Pnt.Y, line.Origin.Y);
                Assert.AreEqual<double>(ifcLine.Dir.Magnitude, line.Direction.Magnitude);
                Assert.AreEqual(ifcLine.Dir.Orientation.X, line.Direction.X);
                Assert.AreEqual(ifcLine.Dir.Orientation.Y, line.Direction.Y);

                var p1 = line.GetPoint(500);
                Assert.AreEqual((500 * ifcLine.Dir.Magnitude) + ifcLine.Pnt.X, p1.X); //The parametric unit (magnitude) is parametricLength, so 500 * parametricLength is the distance
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

        #endregion

        #region Circles

        [DataTestMethod]
        [DataRow(10)]
        [DataRow(-10, false, true)]
        [DataRow(0, false, true)]
        [DataRow(10, true, true)]
        public void Can_convert_ifc_circle_3d(double radius, bool location2d = false, bool checkException = false)
        {
            var ifcCircle = IfcMoq.IfcCircle3dMock(radius: radius, location: location2d ? IfcMoq.IfcIfcAxis2Placement2DMock() : null);
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                if (checkException)
                    Assert.ThrowsException<XbimGeometryFactoryException>(() => factory.Build(ifcCircle));
                else
                {
                    var circle = factory.Build(ifcCircle);
                    Assert.AreEqual(XCurveType.IfcCircle, circle.CurveType);
                    Assert.IsTrue(circle.Is3d);
                }
            }
        }
        [DataTestMethod]
        [DataRow(10)]
        [DataRow(-10, null, true)]
        [DataRow(0, null, true)]
        [DataRow(10, true, true)]
        public void Can_convert_ifc_circle_2d(double radius, bool location3d = false, bool checkException = false)
        {
            var ifcCircle = IfcMoq.IfcCircle2dMock(radius: radius, location: location3d ? IfcMoq.IfcIfcAxis2Placement3DMock() : null);
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                if (checkException)
                    Assert.ThrowsException<XbimGeometryFactoryException>(() => factory.Build(ifcCircle));
                else
                {
                    var circle = factory.Build(ifcCircle);
                    Assert.AreEqual(XCurveType.IfcCircle, circle.CurveType);
                    Assert.IsFalse(circle.Is3d);
                }
            }
        }
        #endregion
        #region Ellipse

        [DataTestMethod]
        [DataRow(10, -1, true)]
        [DataRow(4, 12)]
        [DataRow(5, 5)]
        [DataRow(15, 5)]
        [DataRow(-1, 1, true)]
        public void Can_convert_ifc_ellipse_3d(double major, double minor, bool checkException = false)
        {
            var ifcEllipse = IfcMoq.IfcEllipse3dMock(semi1: major, semi2: minor);
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                if (checkException)
                    Assert.ThrowsException<XbimGeometryFactoryException>(() => factory.Build(ifcEllipse));
                else
                {
                    var ellipse = factory.Build(ifcEllipse) as IXEllipse;
                    Assert.IsNotNull(ellipse);
                    Assert.AreEqual(XCurveType.IfcEllipse, ellipse.CurveType);
                    Assert.IsTrue(ellipse.Is3d);
                    Assert.IsTrue(ellipse.MajorRadius >= ellipse.MinorRadius);
                }
            }
        }
        [DataTestMethod]
        [DataRow(10, -1, true)]
        [DataRow(4, 12)]
        [DataRow(5, 5)]
        [DataRow(15, 5)]
        [DataRow(-1, 1, true)]
        public void Can_convert_ifc_ellipse_2d(double semi1, double semi2, bool checkException = false)
        {
            var ifcEllipse = IfcMoq.IfcEllipse2dMock(semi1: semi1, semi2: semi2);
            using (var scope = Logger.BeginScope($"Entity #5={ifcEllipse.ExpressType.ExpressNameUpper}"))
            {
                using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
                {
                    if (checkException)
                        Assert.ThrowsException<XbimGeometryFactoryException>(() => factory.Build(ifcEllipse));
                    else
                    {
                        var ellipse = factory.Build(ifcEllipse) as IXEllipse;
                        Assert.IsNotNull(ellipse);
                        Assert.AreEqual(XCurveType.IfcEllipse, ellipse.CurveType);
                        Assert.IsFalse(ellipse.Is3d);
                        Assert.IsTrue(ellipse.MajorRadius >= ellipse.MinorRadius);
                    }
                }
            }
        }
        #endregion

        #region Trimmed Curve Tests

        [DataTestMethod]
        [DataRow(1,  0, 10)]
        [DataRow(10, 0, 10)] //Magnitude 10, Length = 100
        [DataRow(10, 10, 100)]
        public void Can_convert_ifc_trimmed_line_3d(double parametricLength = 1, double trim1 = 0, double trim2 = 10)
        {
            var basisLine = IfcMoq.IfcLine3dMock(
                magnitude: parametricLength,
                origin: IfcMoq.IfcCartesianPoint3dMock(0, 0, 0),
                direction: IfcMoq.IfcDirection3dMock(0, 0, 1));
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(basisCurve: basisLine, trimParam1: trim1, trimParam2: trim2);
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var tc = factory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
                Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
                Assert.AreEqual(XCurveType.IfcLine, tc.BasisCurve.CurveType);
                Assert.IsTrue(tc.Is3d);
                Assert.AreEqual(tc.EndPoint.Z, parametricLength * trim2);
                Assert.AreEqual(tc.StartPoint.Z, parametricLength * trim1);
            }

        }

        [DataTestMethod]
        [DataRow(1, 0, 10)]
        [DataRow(10, 0, 10)] //Magnitude 10, Length = 100
        [DataRow(10, 10, 100)]
        public void Can_convert_ifc_trimmed_line_2d(double parametricLength = 1, double trim1 = 0, double trim2 = 10)
        {
            var basisLine = IfcMoq.IfcLine2dMock(
                  magnitude: parametricLength,
                  origin: IfcMoq.IfcCartesianPoint2dMock(0, 0),
                  direction: IfcMoq.IfcDirection2dMock(1, 0));
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve2dMock(basisCurve: basisLine, trimParam1: trim1, trimParam2: trim2);
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var tc = factory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
                Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
                Assert.AreEqual(XCurveType.IfcLine, tc.BasisCurve.CurveType);
                Assert.IsFalse(tc.Is3d);
                //the default trim params are 0, 1, the dwfaults for the model are radian so 1 rad for the trim
                Assert.AreEqual(tc.EndPoint.X, parametricLength * trim2 );
                Assert.AreEqual(tc.StartPoint.X, parametricLength * trim1);
            }

        }

        [TestMethod]
        public void Can_convert_ifc_trimmed_circle_3d()
        {
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(IfcMoq.IfcCircle3dMock(), trimParam2: Math.PI / 2.0);
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var tc = factory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
                Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
                Assert.AreEqual(XCurveType.IfcCircle, tc.BasisCurve.CurveType);
                Assert.IsTrue(tc.Is3d);

            }
        }
        [TestMethod]
        public void Can_convert_ifc_trimmed_circle_2d()
        {
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve2dMock(IfcMoq.IfcCircle2dMock());
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var tc = factory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
                Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
                Assert.AreEqual(XCurveType.IfcCircle, tc.BasisCurve.CurveType);
                Assert.IsFalse(tc.Is3d);
            }
        }

        [DataTestMethod]
        [DataRow(10, 5)]
        [DataRow(5, 10)]
        public void Can_convert_ifc_trimmed_ellipse_3d(double semi1 = 10, double semi2 = 5)
        {
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(
                trimParam1: 0,
                trimParam2: Math.PI / 2.0,
                basisCurve: IfcMoq.IfcEllipse3dMock(semi1: semi1, semi2:semi2));
            var model = IfcMoq.IfcModelMock();
            using (var factory = new CurveFactory(LoggingService, model))
            {
                var tc = factory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
                Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
                Assert.AreEqual(XCurveType.IfcEllipse, tc.BasisCurve.CurveType);
                var basisCurve = tc.BasisCurve as IXEllipse;
                Assert.IsNotNull(basisCurve);
                var origin = basisCurve.Position as IXAxis2Placement3d;
                Assert.IsNotNull(origin);
                Assert.IsTrue(tc.Is3d);
                Assert.AreEqual(tc.StartPoint.X, origin.Axis.Location.X + semi1, model.ModelFactors.Precision);
                Assert.AreEqual(tc.StartPoint.Y, origin.Axis.Location.Y, model.ModelFactors.Precision);
                Assert.AreEqual(tc.EndPoint.X, origin.Axis.Location.X, model.ModelFactors.Precision );
                Assert.AreEqual(tc.EndPoint.Y, origin.Axis.Location.Y + semi2, model.ModelFactors.Precision);
            }
        }
        [DataTestMethod]
        [DataRow(10, 5)]
        [DataRow(5, 10)]
        public void Can_convert_ifc_trimmed_ellipse_2d(double semi1 = 10, double semi2 = 5)
        {
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve2dMock(
                trimParam1: 0,
                trimParam2: Math.PI / 2.0,
                basisCurve: IfcMoq.IfcEllipse2dMock(semi1: semi1, semi2: semi2));
            var model = IfcMoq.IfcModelMock();
            using (var factory = new CurveFactory(LoggingService, IfcMoq.IfcModelMock()))
            {
                var tc = factory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
                Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
                Assert.AreEqual(XCurveType.IfcEllipse, tc.BasisCurve.CurveType);
                var basisCurve = tc.BasisCurve as IXEllipse;
                Assert.IsNotNull(basisCurve);
                var origin = basisCurve.Position as IXAxis2Placement2d;
                Assert.IsNotNull(origin);
                Assert.IsFalse(tc.Is3d);
                Assert.AreEqual(tc.StartPoint.X, origin.Location.X + semi1, model.ModelFactors.Precision);
                Assert.AreEqual(tc.StartPoint.Y, origin.Location.Y, model.ModelFactors.Precision);
                Assert.AreEqual(tc.EndPoint.X, origin.Location.X, model.ModelFactors.Precision);
                Assert.AreEqual(tc.EndPoint.Y, origin.Location.Y + semi2, model.ModelFactors.Precision);
            }
        }
        #endregion


    }
}
