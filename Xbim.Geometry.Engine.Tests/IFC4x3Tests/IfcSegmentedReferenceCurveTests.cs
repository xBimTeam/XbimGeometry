using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcSegmentedReferenceCurveTests
    {
        private const double Precision = 1e-3;
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;


        public IfcSegmentedReferenceCurveTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }


        [Theory]
        [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc")]
        public void CanBuildIfcSegmentedReferenceCurve(string filePath)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var curve = model.Instances.FirstOrDefault(g => g is IfcSegmentedReferenceCurve) as IfcSegmentedReferenceCurve;
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var xCurve = modelSvc.CurveFactory.Build(curve);

            // Assert
            xCurve.Should().NotBeNull();
        }

        [Theory]
        [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc")]
        public void IfNoSuperElevationIfcSegmentedReferenceCurveIsSameAsBaseCurve(string filePath)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var curve = model.Instances.FirstOrDefault(g => g is IfcSegmentedReferenceCurve) as IfcSegmentedReferenceCurve;
            var baseCurve = model.Instances.FirstOrDefault(g => g is IfcGradientCurve) as IfcGradientCurve;
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var xCurve = modelSvc.CurveFactory.Build(curve);
            var xBaseCurve = modelSvc.CurveFactory.Build(baseCurve);

            // Assert
            xCurve.Should().NotBeNull();
            xBaseCurve.Should().NotBeNull();
            xCurve.FirstParameter.Should().BeApproximately(xBaseCurve.FirstParameter, Precision);
            xCurve.LastParameter.Should().BeApproximately(xBaseCurve.LastParameter, Precision);

            for (int u = (int)xCurve.FirstParameter; u < (int)xCurve.LastParameter; u++)
            {
                var p1 = xCurve.GetPoint(u);
                var p2 = xBaseCurve.GetPoint(u);
                p1.X.Should().BeApproximately(p2.X, Precision);
                p1.Y.Should().BeApproximately(p2.Y, Precision);
                p1.Z.Should().BeApproximately(p2.Z, Precision);
            }
        }

    }
}
