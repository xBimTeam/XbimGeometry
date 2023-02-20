
using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{

    public class CurveFactoryTests
    {

        #region Setup

        static ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;
        private readonly IXbimGeometryServicesFactory factory;

        public CurveFactoryTests(IXbimGeometryServicesFactory factory)
        {
            this.factory = factory;
            _modelSvc = factory.CreateModelGeometryService(_dummyModel, loggerFactory);
        }

        #endregion
        public struct PolyData
        {
            public string Name;
            public int EdgeCount;
            public (double X, double Y)[] Points;
        }

        #region Line Tests

        [Theory]
        [InlineData(1)]
        [InlineData(10)]
        [InlineData(-10)]
        [InlineData(0.1)]
        public void Can_convert_ifc_line_3d(double parametricLength)
        {
          
            var ifcLine = IfcMoq.IfcLine3dMock(magnitude: parametricLength);
            var ifcLineSegment = IfcMoq.IfcTrimmedCurve3dMock(ifcLine, 0, 100);//when we trim a line, the magnitude is considered to meet IFC rules, if magnitude is 0.1 then the last parameter will be 0.1 * 100=10

            var curveFactory = _modelSvc.CurveFactory;
            var line = curveFactory.Build(ifcLine) as IXLine; //initialise the factory with the line
            var trimmedLine = curveFactory.Build(ifcLineSegment) as IXTrimmedCurve;
            Assert.NotNull(line);
            Assert.Equal(ifcLine.Pnt.X, line.Origin.X);
            Assert.Equal(ifcLine.Pnt.Y, line.Origin.Y);
            Assert.Equal(ifcLine.Pnt.Z, line.Origin.Z);
            trimmedLine.LastParameter.Should().Be(Math.Abs(parametricLength * 100));
            trimmedLine.EndPoint.Z.Should().Be(parametricLength * 100);
            Assert.Equal(ifcLine.Dir.Orientation.X, line.Direction.X);
            Assert.Equal(ifcLine.Dir.Orientation.Y, line.Direction.Y);
            Assert.Equal(ifcLine.Dir.Orientation.Z, line.Direction.Z);
            var p1 = line.GetPoint(500);
            
            var p2 = line.GetFirstDerivative(500, out IXDirection normal);
            Assert.Equal(p1.X, p2.X);
            Assert.Equal(p1.Y, p2.Y);
            Assert.Equal(p1.Z, p2.Z);
            Assert.Equal(line.Direction.X, normal.X);
            Assert.Equal(line.Direction.Y, normal.Y);
            Assert.Equal(line.Direction.Z, normal.Z);
            Assert.True(line.Is3d);

        }

        [Theory]
        [InlineData(1)]
        [InlineData(10)]
        [InlineData(-10)]
        [InlineData(0.1)]
        public void Can_convert_ifc_line_2d(double parametricLength)
        {
            
            var ifcLine = IfcMoq.IfcLine2dMock(magnitude: parametricLength);
            var ifcLineSegment = IfcMoq.IfcTrimmedCurve2dMock(ifcLine, 0, 100);//when we trim a line, the magnitude is considered to meet IFC rules, if magnitude is 0.1 then the last parameter will be 0.1 * 100=10
            var curveFactory = _modelSvc.CurveFactory;
            var line = curveFactory.Build(ifcLine) as IXLine; //initialise the factory with the line
            var trimmedLine = curveFactory.Build(ifcLineSegment) as IXTrimmedCurve;
            Assert.NotNull(line);
            Assert.Equal(ifcLine.Pnt.X, line.Origin.X);
            Assert.Equal(ifcLine.Pnt.Y, line.Origin.Y);
            Assert.Equal<double>(ifcLine.Dir.Magnitude, line.Direction.Magnitude);
            Assert.Equal(ifcLine.Dir.Orientation.X, line.Direction.X);
            Assert.Equal(ifcLine.Dir.Orientation.Y, line.Direction.Y);

            trimmedLine.LastParameter.Should().Be(Math.Abs(parametricLength* 100));

            trimmedLine.EndPoint.X.Should().Be(parametricLength * 100);
            var p1 = line.GetPoint(500);
            var p2 = line.GetFirstDerivative(500, out IXDirection normal);
            Assert.Equal(p1.X, p2.X);
            Assert.Equal(p1.Y, p2.Y);

            Assert.Equal(line.Direction.X, normal.X);
            Assert.Equal(line.Direction.Y, normal.Y);

            Assert.False(line.Is3d);
            p1.Invoking(p => p.Z).Should().Throw<Exception>();
            normal.Invoking(n => n.Z).Should().Throw<Exception>();

        }

        #endregion

        #region Circles

        [Theory]
        [InlineData(10)]
        [InlineData(-10, false, true)]
        [InlineData(0, false, true)]
        [InlineData(10, true)]
        public void Can_convert_ifc_circle_3d(double radius, bool location2d = false, bool checkException = false)
        {
           
            var ifcCircle = IfcMoq.IfcCircle3dMock(radius: radius, location: location2d ? IfcMoq.IfcAxis2Placement2DMock() : null);
            var curveFactory = _modelSvc.CurveFactory;
            if (checkException)
                ifcCircle.Invoking(c => curveFactory.Build(c)).Should().Throw<Exception>();

            else
            {
                var circle = curveFactory.Build(ifcCircle);
                Assert.Equal(XCurveType.IfcCircle, circle.CurveType);
                Assert.True(circle.Is3d);
            }

        }
        [Theory]
        [InlineData(10)]
        [InlineData(-10, false, true)]
        [InlineData(0, false, true)]
        [InlineData(10, true, true)]
        public void Can_convert_ifc_circle_2d(double radius, bool location3d = false, bool checkException = false)
        {
           
            var ifcCircle = IfcMoq.IfcCircle2dMock(radius: radius, location: location3d ? IfcMoq.IfcAxis2Placement3DMock() : null);
            var curveFactory = _modelSvc.CurveFactory;
            if (checkException)
                ifcCircle.Invoking(c => curveFactory.Build(c)).Should().Throw<Exception>();
            else
            {
                var circle = curveFactory.Build(ifcCircle);
                Assert.Equal(XCurveType.IfcCircle, circle.CurveType);
                Assert.False(circle.Is3d);
            }
        }
        #endregion
        #region Ellipse

        [Theory]
        [InlineData(10, -1, true)]
        [InlineData(4, 12)]
        [InlineData(5, 5)]
        [InlineData(15, 5)]
        [InlineData(-1, 1, true)]
        public void Can_convert_ifc_ellipse_3d(double major, double minor, bool checkException = false)
        {
           
            var ifcEllipse = IfcMoq.IfcEllipse3dMock(semi1: major, semi2: minor);
            var curveFactory = _modelSvc.CurveFactory;
            if (checkException)
                ifcEllipse.Invoking(c => curveFactory.Build(c)).Should().Throw<Exception>();
            else
            {
                var ellipse = curveFactory.Build(ifcEllipse) as IXEllipse;
                Assert.NotNull(ellipse);
                Assert.Equal(XCurveType.IfcEllipse, ellipse.CurveType);
                Assert.True(ellipse.Is3d);
                Assert.True(ellipse.MajorRadius >= ellipse.MinorRadius);
            }
        }
        [Theory]
        [InlineData(10, -1, true)]
        [InlineData(4, 12)]
        [InlineData(5, 5)]
        [InlineData(15, 5)]
        [InlineData(-1, 1, true)]
        public void Can_convert_ifc_ellipse_2d(double semi1, double semi2, bool checkException = false)
        {
           
            var ifcEllipse = IfcMoq.IfcEllipse2dMock(semi1: semi1, semi2: semi2);
            var curveFactory = _modelSvc.CurveFactory;
            if (checkException)
                ifcEllipse.Invoking(c => curveFactory.Build(c)).Should().Throw<Exception>();
            else
            {
                var ellipse = curveFactory.Build(ifcEllipse) as IXEllipse;
                Assert.NotNull(ellipse);
                Assert.Equal(XCurveType.IfcEllipse, ellipse.CurveType);
                Assert.False(ellipse.Is3d);
                Assert.True(ellipse.MajorRadius >= ellipse.MinorRadius);
            }
        }


        #endregion

        #region Trimmed Curve Tests

        [Theory]
        [InlineData(1, 0, 10)]
        [InlineData(10, 0, 10)] //Magnitude 10, Length = 100
        [InlineData(10, 10, 100)]
        public void Can_convert_ifc_trimmed_line_3d(double parametricLength = 1, double trim1 = 0, double trim2 = 10)
        {
          
            var basisLine = IfcMoq.IfcLine3dMock(
            magnitude: parametricLength,
            origin: IfcMoq.IfcCartesianPoint3dMock(0, 0, 0),
            direction: IfcMoq.IfcDirection3dMock(0, 0, 1));
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(basisCurve: basisLine, trimParam1: trim1, trimParam2: trim2);
            var curveFactory = _modelSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
            Assert.Equal(XCurveType.IfcTrimmedCurve, tc.CurveType);
            Assert.Equal(XCurveType.IfcLine, tc.BasisCurve.CurveType);
            Assert.True(tc.Is3d);
            Assert.Equal(tc.EndPoint.Z, parametricLength * trim2);
            Assert.Equal(tc.StartPoint.Z, parametricLength * trim1);
        }

        [Theory]
        [InlineData(1, 0, 10)]
        [InlineData(10, 0, 10)] //Magnitude 10, Length = 100
        [InlineData(10, 10, 100)]
        public void Can_convert_ifc_trimmed_line_2d(double parametricLength = 1, double trim1 = 0, double trim2 = 10)
        {
          
            var basisLine = IfcMoq.IfcLine2dMock(
              magnitude: parametricLength,
              origin: IfcMoq.IfcCartesianPoint2dMock(0, 0),
              direction: IfcMoq.IfcDirection2dMock(1, 0));
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve2dMock(basisCurve: basisLine, trimParam1: trim1, trimParam2: trim2);
            var curveFactory = _modelSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
            Assert.Equal(XCurveType.IfcTrimmedCurve, tc.CurveType);
            Assert.Equal(XCurveType.IfcLine, tc.BasisCurve.CurveType);
            Assert.False(tc.Is3d);
            //the default trim params are 0, 1, the dwfaults for the model are radian so 1 rad for the trim
            Assert.Equal(tc.EndPoint.X, parametricLength * trim2);
            Assert.Equal(tc.StartPoint.X, parametricLength * trim1);
        }
        //
        //  [TestMethod]
        //public void Can_convert_ifc_trimmed_circle_3d()
        //{
        //    var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(IfcMoq.IfcCircle3dMock(), trimParam2: Math.PI / 2.0);
        //    var curveFactory = _modelScope.ServiceProvider.GetRequiredService<IXCurveService>();
        //    var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
        //    Assert.AreEqual(XCurveType.IfcTrimmedCurve, tc.CurveType);
        //    Assert.AreEqual(XCurveType.IfcCircle, tc.BasisCurve.CurveType);
        //    Assert.IsTrue(tc.Is3d);


        //}
        [Theory] //see the IFC definition https://standards.buildingsmart.org/IFC/RELEASE/IFC4/ADD2/HTML/link/ifctrimmedcurve.htm
        [InlineData(90, 180, true, 1)] //Ifc Case 1
        [InlineData(180, 90, true, 2)] //Ifc Case 2
        [InlineData(180, 90, false, 3)]//Ifc Case 3
        [InlineData(90, 180, false, 4)]//Ifc Case 4
        [InlineData(12, 0, false, 5)]//custom

        public void Can_convert_ifc_trimmed_circle_2d(double trim1, double trim2, bool sameSense, int ifcCase)
        {
            double radius = 10;
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve2dMock(
                trimParam1: trim1,
                trimParam2: trim2,
                sense: sameSense,
                basisCurve: IfcMoq.IfcCircle2dMock(radius: radius));

            var curveFactory = _modelSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
            Assert.Equal(XCurveType.IfcTrimmedCurve, tc.CurveType);
            Assert.Equal(XCurveType.IfcCircle, tc.BasisCurve.CurveType);
            var basisCurve = tc.BasisCurve as IXCircle;
            Assert.NotNull(basisCurve);
            var origin = basisCurve.Position as IXAxisPlacement2d;
            Assert.NotNull(origin);
           
            switch (ifcCase)
            {
                case 1:
                    (radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    tc.StartPoint.Y.Should().BeApproximately(radius, _modelSvc.Precision);
                    tc.EndPoint.X.Should().BeApproximately(-radius, _modelSvc.Precision);

                    break;
                case 2:
                    (3 * radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    tc.EndPoint.Y.Should().BeApproximately(radius, _modelSvc.Precision);
                    tc.StartPoint.X.Should().BeApproximately(-radius, _modelSvc.Precision);
                    break;
                case 3:
                    (radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    tc.EndPoint.Y.Should().BeApproximately(radius, _modelSvc.Precision);
                    tc.StartPoint.X.Should().BeApproximately(-radius, _modelSvc.Precision);
                    break;
                case 4:
                    (3 * radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    tc.StartPoint.Y.Should().BeApproximately(radius, _modelSvc.Precision);
                    tc.EndPoint.X.Should().BeApproximately(-radius, _modelSvc.Precision);
                    break;
                case 5:
                    tc.Length.Should().BeApproximately(radius * trim1 * Math.PI / 180, _modelSvc.Precision);
                    tc.StartPoint.X.Should().BeApproximately(radius*Math.Cos(trim1 * Math.PI / 180), _modelSvc.Precision);
                    tc.StartPoint.Y.Should().BeApproximately(radius * Math.Sin(trim1 * Math.PI / 180), _modelSvc.Precision);
                    tc.EndPoint.X.Should().BeApproximately(radius, _modelSvc.Precision);
                    tc.EndPoint.Y.Should().BeApproximately(0, _modelSvc.Precision);
                    break;
                default:
                    break;
            }
            tc.Is3d.Should().BeFalse();
        }

        [Theory] //see the IFC definition https://standards.buildingsmart.org/IFC/RELEASE/IFC4/ADD2/HTML/link/ifctrimmedcurve.htm
        [InlineData(90, 180, true, 1)] //Ifc Case 1
        [InlineData(180, 90, true, 2)] //Ifc Case 2
        [InlineData(180, 90, false, 3)]//Ifc Case 3
        [InlineData(90, 180, false, 4)]//Ifc Case 4
        public void Can_convert_ifc_trimmed_circle_3d(double trim1, double trim2, bool sameSense, int ifcCase)
        {
           
            double radius = 10;
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(
                trimParam1: trim1,
                trimParam2: trim2,
                sense: sameSense,
                basisCurve: IfcMoq.IfcCircle3dMock(radius: radius));

            var curveFactory = _modelSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
            Assert.Equal(XCurveType.IfcTrimmedCurve, tc.CurveType);
            Assert.Equal(XCurveType.IfcCircle, tc.BasisCurve.CurveType);
            var basisCurve = tc.BasisCurve as IXCircle;
            Assert.NotNull(basisCurve);
            var origin = basisCurve.Position as IXAxis2Placement3d;
            Assert.NotNull(origin);
            Assert.True(tc.Is3d);

            switch (ifcCase)
            {
                case 1:
                    (radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    (tc.StartPoint.Y).Should().BeApproximately(radius, _modelSvc.Precision);
                    (tc.EndPoint.X).Should().BeApproximately(-radius, _modelSvc.Precision);

                    break;
                case 2:
                    (3 * radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    (tc.EndPoint.Y).Should().BeApproximately(radius, _modelSvc.Precision);
                    (tc.StartPoint.X).Should().BeApproximately(-radius, _modelSvc.Precision);
                    break;
                case 3:
                    (radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    (tc.EndPoint.Y).Should().BeApproximately(radius, _modelSvc.Precision);
                    (tc.StartPoint.X).Should().BeApproximately(-radius, _modelSvc.Precision);
                    break;
                case 4:
                    (3 * radius * Math.PI / 2).Should().BeApproximately(tc.Length, _modelSvc.Precision);
                    (tc.StartPoint.Y).Should().BeApproximately(radius, _modelSvc.Precision);
                    (tc.EndPoint.X).Should().BeApproximately(-radius, _modelSvc.Precision);
                    break;
                default:
                    break;
            }
        }


        [Theory]

        [InlineData(90, 180, true, true)]
        [InlineData(180, 90, true, true)]
        [InlineData(180, 90, true, false)]
        [InlineData(90, 180, true, false)]
        [InlineData(90, 180, false, true)]
        [InlineData(180, 90, false, true)]
        [InlineData(180, 90, false, false)]
        [InlineData(90, 180, false, false)]

        // [DataRow(5, 10, true)]
        public void Can_convert_ifc_trimmed_ellipse_3d(double trim1, double trim2, bool reverseAxis, bool sameSense = true)
        {
          
            double semi1 = reverseAxis ? 5 : 10;
            double semi2 = reverseAxis ? 10 : 5;
            double quadrantLength = 12.110560271815889;
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve3dMock(
                trimParam1: trim1,
                trimParam2: trim2,
                sense: sameSense,
                basisCurve: IfcMoq.IfcEllipse3dMock(semi1: semi1, semi2: semi2));
            
            var curveFactory = _modelSvc.CurveFactory;
            var tc = curveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
            Assert.Equal(XCurveType.IfcTrimmedCurve, tc.CurveType);
            Assert.Equal(XCurveType.IfcEllipse, tc.BasisCurve.CurveType);
            var basisCurve = tc.BasisCurve as IXEllipse;
            Assert.NotNull(basisCurve);
            var origin = basisCurve.Position as IXAxis2Placement3d;
            Assert.NotNull(origin);
            Assert.True(tc.Is3d);
            var sp = tc.StartPoint;
            var ep = tc.EndPoint;
            if (sameSense)
            {
                if (trim1 > trim2)
                {
                    (3 * quadrantLength).Should().BeApproximately(tc.Length, 1e-2);
                    (tc.StartPoint.X).Should().BeApproximately(origin.Axis.Location.X - semi1, _modelSvc.Precision);
                    (tc.EndPoint.Y).Should().BeApproximately(origin.Axis.Location.Y + semi2, _modelSvc.Precision);
                }
                else
                {
                    (quadrantLength).Should().BeApproximately(tc.Length, 1e-2);
                    (tc.StartPoint.Y).Should().BeApproximately(origin.Axis.Location.Y + semi2, _modelSvc.Precision);
                    (tc.EndPoint.X).Should().BeApproximately(origin.Axis.Location.X - semi1, _modelSvc.Precision);
                }
            }
            else
            {
                if (trim1 > trim2)
                {
                    (3 * quadrantLength).Should().BeApproximately(tc.Length, 1e-2);
                    (tc.StartPoint.X).Should().BeApproximately(origin.Axis.Location.X - semi1, _modelSvc.Precision);
                    (tc.EndPoint.Y).Should().BeApproximately(origin.Axis.Location.Y + semi2, _modelSvc.Precision);
                }
                else
                {
                    (quadrantLength).Should().BeApproximately(tc.Length, 1e-2);
                    (tc.StartPoint.Y).Should().BeApproximately(origin.Axis.Location.Y + semi2, _modelSvc.Precision);
                    (tc.EndPoint.X).Should().BeApproximately(origin.Axis.Location.X - semi1, _modelSvc.Precision);
                }
            }
        }
        [Theory]
        [InlineData(10, 5)]
        [InlineData(5, 10)]
        public void Can_convert_ifc_trimmed_ellipse_2d(double semi1 = 10, double semi2 = 5)
        {
           
            var ifcTrimmedCurve = IfcMoq.IfcTrimmedCurve2dMock(
            trimParam1: 0,
            trimParam2: 90,
            basisCurve: IfcMoq.IfcEllipse2dMock(semi1: semi1, semi2: semi2));

            var tc = _modelSvc.CurveFactory.Build(ifcTrimmedCurve) as IXTrimmedCurve; //initialise the factory with the curve
            Assert.NotNull(tc);
            Assert.Equal(XCurveType.IfcTrimmedCurve, tc.CurveType);
            Assert.Equal(XCurveType.IfcEllipse, tc.BasisCurve.CurveType);
            var basisCurve = tc.BasisCurve as IXEllipse;
            Assert.NotNull(basisCurve);
            var origin = basisCurve.Position as IXAxisPlacement2d;
            Assert.NotNull(origin);
            Assert.False(tc.Is3d);
            (tc.StartPoint.X).Should().BeApproximately(origin.Location.X + semi1, _modelSvc.Precision);
            (tc.StartPoint.Y).Should().BeApproximately(origin.Location.Y, _modelSvc.Precision);
            (tc.EndPoint.X).Should().BeApproximately(origin.Location.X, _modelSvc.Precision);
            (tc.EndPoint.Y).Should().BeApproximately(origin.Location.Y + semi2, _modelSvc.Precision);
        }
        #endregion

        #region Composite Curves
        [Fact]
        public void Can_convert_ifc_composite_curve_simple_arc()
        {
           
            var ifcCompCurve = IfcMoq.IfcCompositeCurve3dMock();
            var curveService = _modelSvc.CurveFactory;
            var edgeService = _modelSvc.EdgeFactory;
            var cc = curveService.Build(ifcCompCurve) as IXBSplineCurve; //initialise the factory with the curve
            Assert.NotNull(cc);
            Assert.Equal(XCurveType.IfcCompositeCurve, cc.CurveType);
            var edge = edgeService.Build(cc);
#if DEBUG
            var str = edge.BrepString();
#endif
            cc.IsPeriodic.Should().BeFalse();
            cc.IsRational.Should().BeTrue();
            var paramsRads = cc.LastParameter - cc.FirstParameter;

            paramsRads.Should().BeApproximately(Math.PI * 0.5, 1e-5); //parametric length in radians of this curve is 90 degrees
        }
        [Fact]
        public void Can_convert_ifc_composite_curve_three_arcs()
        {
          
            
            var circ1 = IfcMoq.IfcCircle3dMock(radius: 20);
            var circ2 = IfcMoq.IfcCircle3dMock(radius: 20, IfcMoq.IfcAxis2Placement3DMock(refDir: IfcMoq.IfcDirection3dMock(-1, 0, 0), loc: IfcMoq.IfcCartesianPoint3dMock(0, 40, 0)));
            var circ3 = IfcMoq.IfcCircle3dMock(radius: 20, IfcMoq.IfcAxis2Placement3DMock(loc: IfcMoq.IfcCartesianPoint3dMock(-40, 40, 0)));


            var arc1 = IfcMoq.IfcTrimmedCurve3dMock(circ1, trimParam2: 90);
            var arc2 = IfcMoq.IfcTrimmedCurve3dMock(circ2, trimParam1: 90, trimParam2: 0, sense: true);
            var arc3 = IfcMoq.IfcTrimmedCurve3dMock(circ3, trimParam2: 90);

            var x1 = _modelSvc.CurveFactory.Build(arc1) as IXTrimmedCurve;
            var x2 = _modelSvc.CurveFactory.Build(arc2) as IXTrimmedCurve;
            var x3 = _modelSvc.CurveFactory.Build(arc3) as IXTrimmedCurve;
            var paramLength1 = x1.LastParameter - x1.FirstParameter;
            var paramLength2 = x2.LastParameter - x2.FirstParameter;
            var paramLength3 = x3.LastParameter - x3.FirstParameter;
            var totalParametricLength = paramLength1 + paramLength2 + paramLength3;
            var seg1 = IfcMoq.IfcCompositeCurveSegment3dMock(arc1, entityLabel: 1);
            var seg2 = IfcMoq.IfcCompositeCurveSegment3dMock(arc2, entityLabel: 2);
            var seg3 = IfcMoq.IfcCompositeCurveSegment3dMock(arc3, entityLabel: 3);

            var ifcCompCurve = IfcMoq.IfcCompositeCurve3dMock(new[] { seg1, seg2, seg3 });

            var cc = _modelSvc.CurveFactory.Build(ifcCompCurve) as IXBSplineCurve; //initialise the factory with the curve
            Assert.NotNull(cc);
            Assert.Equal(XCurveType.IfcCompositeCurve, cc.CurveType);
            (totalParametricLength).Should().BeApproximately(cc.LastParameter - cc.FirstParameter, _modelSvc.Precision); //parametric length of this curve is 90 degrees

        }
        [Fact]
        public void Can_convert_ifc_composite_curve_three_arcs_two_lines()
        {
           
     
            var ifcCompCurve = IfcMoq.TypicalCompositeCurveMock(_modelSvc.CurveFactory, out double totalParametricLength, out double totalLength);

            var cc = _modelSvc.CurveFactory.Build(ifcCompCurve) as IXBSplineCurve; //initialise the factory with the curve
            Assert.NotNull(cc);
            Assert.Equal(XCurveType.IfcCompositeCurve, cc.CurveType);
            (totalLength).Should().BeApproximately(cc.Length, _modelSvc.MinimumGap);
            (totalParametricLength).Should().BeApproximately(cc.LastParameter - cc.FirstParameter, _modelSvc.Precision); //parametric length of this curve is 90 degrees
        }

        [Fact]
        public void Can_convert_ifc_composite_curve_to_directrix()
        {
           
            var ifcCompCurve = IfcMoq.TypicalCompositeCurveMock(_modelSvc.CurveFactory, out double totalParametricLength, out double totalLength);

            var cc = _modelSvc.CurveFactory.BuildDirectrix(ifcCompCurve, 10, totalParametricLength - 10) as IXBSplineCurve; //initialise the factory with the curve
            Assert.NotNull(cc);
            Assert.Equal(XCurveType.IfcCompositeCurve, cc.CurveType);
            (totalParametricLength - 20).Should().BeApproximately(cc.LastParameter - cc.FirstParameter, _modelSvc.Precision); //parametric length of this curve is 90 degrees
        }

        #endregion
    }
}
