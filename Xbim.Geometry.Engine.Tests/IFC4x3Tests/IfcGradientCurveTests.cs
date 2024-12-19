using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3;
using Xbim.Ifc4x3.GeometricModelResource;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3.MeasureResource;
using Xbim.Ifc4x3.ProfileResource;
using Xbim.IO.Memory;
using Xunit;
using ILoggerFactory = Microsoft.Extensions.Logging.ILoggerFactory;



namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcGradientCurveTests
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;


        public IfcGradientCurveTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }


        [Theory]
        [InlineData(@"TestFiles\IFC4x3\test.ifc")]
        [InlineData(@"TestFiles\IFC4x3\Viadotto Acerno.ifc")]
        [InlineData(@"TestFiles\IFC4x3\SectionedSolidHorizontal-1.ifc")]
        [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc")]
        public void CanBuildIfcGradientCurve(string filePath)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var curve = model.Instances.FirstOrDefault(g => g is IfcGradientCurve) as IfcGradientCurve;
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var xCurve = modelSvc.CurveFactory.Build(curve);

            // Assert
            xCurve.Should().NotBeNull();
        }

    }

}
