using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3.MeasureResource;
using Xbim.IO.Memory;
using Xunit;
using ILoggerFactory = Microsoft.Extensions.Logging.ILoggerFactory;

namespace Xbim.Geometry.Engine.Tests
{
    public class IfcPointByDistanceExpressionTests
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;
        private const double Tolerance = 1e-5;


        public IfcPointByDistanceExpressionTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }


        [Theory]
        [InlineData(20, 94, XCurveMeasureType.LengthMeasure, Math.PI / 2)]
        [InlineData(20, 180, XCurveMeasureType.LengthMeasure, Math.PI)]
        [InlineData(23, 80, XCurveMeasureType.LengthMeasure, Math.PI / 10)]
        [InlineData(30, 300, XCurveMeasureType.ParameterValue, 0.2)]
        public void CanBuildIfcCartesianPointFromIfcPointByDistanceExpression
                (double radius, double semiCircleEndAngle, XCurveMeasureType curveMeasureType, double curveMeasure)
        {
            // Arrange
            IfcCurveMeasureSelect distanceAlong = curveMeasureType switch
            {
                XCurveMeasureType.LengthMeasure => new IfcLengthMeasure(radius * curveMeasure),
                XCurveMeasureType.ParameterValue => new IfcParameterValue(curveMeasure),
                _ => throw new NotImplementedException()
            };

            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildIfcCartesianPointFromIfcPointByDistanceExpression));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);
            IfcPointByDistanceExpression pointByDistanceExpression = CreatePointByDistanceExpression
                                                        (radius, semiCircleEndAngle, distanceAlong, model);

            // Act
            var point = modelSvc.GeometryFactory.BuildPoint3d(pointByDistanceExpression);

            // Assert
            point.Should().NotBeNull();
            var theta = curveMeasureType switch
            {
                XCurveMeasureType.LengthMeasure => (double)(distanceAlong.Value) / radius,
                XCurveMeasureType.ParameterValue => (double)(distanceAlong.Value) * semiCircleEndAngle * (Math.PI / 180),
                _ => throw new NotImplementedException()
            };
            point.X.Should().BeApproximately(radius * Math.Cos(theta), Tolerance);
            point.Y.Should().BeApproximately(radius * Math.Sin(theta), Tolerance);
        }


        private static IfcPointByDistanceExpression CreatePointByDistanceExpression
                    (double radius, double semiCircleEndAngle, IfcCurveMeasureSelect distanceAlong, MemoryModel model)
        {
            var pointByDistanceExpression = model.Instances.New<IfcPointByDistanceExpression>(point =>
            {
                point.DistanceAlong = distanceAlong;
                point.BasisCurve = model.Instances.New<IfcTrimmedCurve>(curve =>
                {
                    curve.BasisCurve = model.Instances.New<IfcCircle>(circle =>
                    {
                        circle.Radius = radius;
                        circle.Position = model.Instances.New<IfcAxis2Placement3D>(placement =>
                        {

                            placement.Axis = model.Instances.New<IfcDirection>(dir =>
                            {
                                dir.X = 0;
                                dir.Y = 0;
                                dir.Z = 1;
                            });
                            placement.RefDirection = model.Instances.New<IfcDirection>(refDir =>
                            {
                                refDir.X = 1;
                                refDir.Y = 0;
                                refDir.Z = 0;
                            });
                            placement.Location = model.Instances.New<IfcCartesianPoint>(p =>
                            {
                                p.X = 0;
                                p.Y = 0;
                                p.Z = 0;
                            });
                        });
                    });
                    curve.MasterRepresentation = Ifc4x3.GeometryResource.IfcTrimmingPreference.PARAMETER;
                    curve.SenseAgreement = true;
                    curve.Trim1.Add(new IfcParameterValue(0));
                    curve.Trim2.Add(new IfcParameterValue(semiCircleEndAngle));
                });
            });
            return pointByDistanceExpression;
        }
    }

}
