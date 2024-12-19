using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{

    public class ShapeBinarySerializerTests
    {
        private readonly IXShapeBinarySerializer _shapeBinarySerializer;
        private readonly IXbimGeometryServicesFactory _factory;
        static MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;


        public ShapeBinarySerializerTests(IXbimGeometryServicesFactory factory, ILoggerFactory loggerFactory)
        {
            _factory = factory;
            _modelSvc = factory.CreateModelGeometryService(_dummyModel, loggerFactory);
            _shapeBinarySerializer = _modelSvc.ShapeBinarySerializer;
        }



        [Fact]
        public void CanSerializeShapeToBinary()
        {
            var solidFactory = _modelSvc.SolidFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);


            var binary = _shapeBinarySerializer.ToArray(block);


            binary.Should().NotBeNullOrEmpty();
        }


        [Fact]
        public void CanDeserializeShapeFromBinary()
        {
            var solidFactory = _modelSvc.SolidFactory;
            var blockMoq = IfcMoq.IfcBlockMoq();
            var block = solidFactory.Build(blockMoq);

            var binary = _shapeBinarySerializer.ToArray(block);

            binary.Should().NotBeNullOrEmpty();

            var deserialized = _shapeBinarySerializer.FromArray(binary);

            deserialized.Should().NotBeNull();
            deserialized.Bounds().LenX.Should().BeApproximately(block.Bounds().LenX, 1e-5);
            deserialized.Bounds().LenY.Should().BeApproximately(block.Bounds().LenY, 1e-5);
            deserialized.Bounds().LenZ.Should().BeApproximately(block.Bounds().LenZ, 1e-5);

        }

    }

}

