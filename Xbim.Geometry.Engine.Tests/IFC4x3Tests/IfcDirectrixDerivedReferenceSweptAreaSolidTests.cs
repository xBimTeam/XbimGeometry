using FluentAssertions;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.GeometricModelResource;
using Xbim.Ifc4x3.GeometryResource;
using Xbim.Ifc4x3.SharedInfrastructureElements;
using Xbim.IO.Memory;
using Xbim.ModelGeometry.Scene;
using Xunit;
using ILoggerFactory = Microsoft.Extensions.Logging.ILoggerFactory;



namespace Xbim.Geometry.Engine.Tests.IFC4x3Tests
{
    public class IfcDirectrixDerivedReferenceSweptAreaSolidTests
    {
        private readonly IXbimGeometryServicesFactory _factory;
        private readonly ILoggerFactory _loggerFactory;
        private const double Tolerance = 1e-5;


        public IfcDirectrixDerivedReferenceSweptAreaSolidTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _loggerFactory = loggerFactory;
        }




        [Theory]
        [InlineData(@"TestFiles\IFC4x3\ACCA_sleepers-linear-placement-cant-implicit.ifc", 2778)]
        [InlineData(@"TestFiles\IFC4x3\DirectrixDerivedReferenceSweptAreaSolid-2.ifc", 119)]
        public void CanBuildIfcDirectrixDerivedReferenceSweptAreaSolid(string ifcFile, int solidId)
        {
            // Arrange
            using var model = MemoryModel.OpenRead(ifcFile);
            var solid = model.Instances[solidId] as IfcDirectrixDerivedReferenceSweptAreaSolid;
            var modelSvc = _factory.CreateModelGeometryService(model, _loggerFactory);

            // Act
            var xSolid = modelSvc.SolidFactory.Build(solid);

            // Assert
            xSolid.Should().NotBeNull();
        }
    }

}
