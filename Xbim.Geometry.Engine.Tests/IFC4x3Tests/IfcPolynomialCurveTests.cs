using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3;
using Xbim.IO.Memory;
using Xunit;
using Xbim.Ifc4x3.MeasureResource;

namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcPolynomialCurveTests
    {
        private const double Precision = 1e-5;
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;


        public IfcPolynomialCurveTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }

        [Theory]
        [InlineData(new object[] { 0.0, 1.0 }, new object[] { 0.0, 0.0, 0.0, -3.88888888888889E-6 }, SegmentType.Cubic_300_1000, -142.857142857143, 100)]
        [InlineData(new object[] { 0.0, 1.0 }, new object[] { 0.0, 0.0, 0.0, -5.55555555555556E-6 }, SegmentType.Cubic_300_inf, -100, 100)]
        public void CanBuildIfcPolynomialCurve(object[] coefficientsX, object[] coefficientsY, SegmentType type, double firstParam, double length)
        {
            // Arrange
            using MemoryModel model = new MemoryModel(new EntityFactoryIfc4x3Add2());
            using var txn = model.BeginTransaction(nameof(CanBuildIfcPolynomialCurve));
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            var polyCurve = model.Instances.New<IfcPolynomialCurve>(p =>
            {
                p.Position = model.Instances.New<IfcAxis2Placement2D>(placement =>
                {
                    placement.Location = model.Instances.New<IfcCartesianPoint>(pnt =>
                    {
                        pnt.X = 0;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });

                    placement.RefDirection = model.Instances.New<IfcDirection>(pnt =>
                    {
                        pnt.X = 1;
                        pnt.Y = 0;
                        pnt.Z = 0;
                    });
                });
            });

            polyCurve.CoefficientsX.AddRange(coefficientsX.Select(x => new IfcReal((double)x)));
            polyCurve.CoefficientsY.AddRange(coefficientsY.Select(x => new IfcReal((double)x)));

            // Act
            var xPolyCurve = modelSvc.CurveFactory.BuildPolynomialCurve2d(polyCurve, firstParam, firstParam + length);

            // Assert
            xPolyCurve.Should().NotBeNull();

            foreach (var location in CurveSegmentData.GetPoints(type))
            {
                var point = xPolyCurve.GetPoint(location.Key + xPolyCurve.FirstParameter);
                point.X.Should().BeApproximately(location.Value.X, Precision);
                point.Y.Should().BeApproximately(location.Value.Y, Precision);
            }
        }
    }
}