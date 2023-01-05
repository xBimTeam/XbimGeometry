using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.NetCore.Tests;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Interop.Tests.ModelGeometryServiceTests
{
    public class CollisionDetectionTests
    {
        private readonly IXShapeService _shapeService;
        private readonly IXModelGeometryService _modelGeomService;
        private MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());

        public CollisionDetectionTests(IXShapeService shapeService, IXGeometryConverterFactory geometryConverterFactory, ILoggerFactory loggerFactory)
        {
            _shapeService = shapeService;
            _modelGeomService = geometryConverterFactory.CreateModelGeometryService(_dummyModel, loggerFactory);
        }

        [Fact]
        public void GivenTwoCollidingShapes_IsColliding_ShouldReturnTrue()
        {
            var blockMoq = IfcMoq.IfcBlockMoq();
            var cylinderMoq = IfcMoq.IfcRightCircularCylinderMoq();

            var block = _modelGeomService.SolidFactory.Build(blockMoq);
            var cylinder = _modelGeomService.SolidFactory.Build(cylinderMoq);

            var result = _shapeService.IsColliding(block, cylinder, 0.001);

            result.Should().Be(true);
        }


        [Fact]
        public void GivenTwoNonCollidingShapes_IsColliding_ShouldReturnFalse()
        {
            var blockMoq = IfcMoq.IfcBlockMoq();
            var cylinderMoq = IfcMoq.IfcRightCircularCylinderMoq(
                                IfcMoq.IfcAxis2Placement3DMock
                                    (null, null, IfcMoq.IfcCartesianPoint3dMock(0, 0, 10000)));

            var block = _modelGeomService.SolidFactory.Build(blockMoq);
            var cylinder = _modelGeomService.SolidFactory.Build(cylinderMoq);

            var result = _shapeService.IsColliding(block, cylinder, 0.001);

            result.Should().Be(false);
        }
    }
}
