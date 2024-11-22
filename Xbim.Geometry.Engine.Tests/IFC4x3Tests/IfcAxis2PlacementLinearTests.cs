using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Drawing;
using System.Security.Cryptography;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3;
using Xbim.Ifc4x3.GeometricConstraintResource;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3.MeasureResource;
using Xbim.IO.Memory;
using Xunit;
using ILoggerFactory = Microsoft.Extensions.Logging.ILoggerFactory;

namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcAxis2PlacementLinearTests
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;
        private const double Tolerance = 1e-5;


        public IfcAxis2PlacementLinearTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }

        [Theory]
        [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc", 3021)]
        [InlineData(@"TestFiles\IFC4x3\Viadotto Acerno.ifc", 194771)]
        public void CanBuildLinearPlacement(string filePath, int placementId)
        {

            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var placement = model.Instances[placementId] as IfcLinearPlacement;
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var location = modelSvc.GeometryFactory.BuildLocation(placement);


            // Assert
            location.Should().NotBeNull();
        }


        [Theory]
        [InlineData(@"TestFiles\IFC4x3\PlacmentOfSignal.ifc")]
        public void CanBuildIfcRefrentsLocations(string filePath)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(filePath);
            var referents = model.Instances.OfType<IIfcReferent>()
                .OrderBy(p => (((p.ObjectPlacement as IfcLinearPlacement).RelativePlacement.Location as IfcPointByDistanceExpression).DistanceAlong as IfcMeasureValue).Value);
           
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var locations = new List<IXLocation>();
            foreach (var referent in referents) {

                var location = modelSvc.GeometryFactory.BuildLocation(referent.ObjectPlacement);
                location.Should().NotBeNull();
                locations.Add(location);
            }

            // Assert
            const double tolerance = 1e-3;
            var translations = locations.Select(loc => loc.Translation).ToList();
            for (int i = 1; i < translations.Count; i++)
            {
                var currentZ = translations[i].Z;
                var previousZ = translations[i - 1].Z;
                double difference = currentZ - previousZ;
                bool isDecreasingWithinTolerance = difference <= tolerance;
                isDecreasingWithinTolerance.Should().BeTrue("The gradient curve has a decreasing elevations along");
            }
        }

    }

}
