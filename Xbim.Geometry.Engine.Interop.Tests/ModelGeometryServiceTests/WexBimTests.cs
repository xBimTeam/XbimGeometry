
using Microsoft.Extensions.Logging;
using System.IO;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.NetCore.Tests;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.NetCore.Tests
{
    public class WexBimTests
    {
        private readonly IXShapeService _shapeService;
        private readonly IXGeometryConverterFactory _geomConverterFactory;
        private readonly ILoggerFactory _loggerFactory;

        public WexBimTests(IXShapeService shapeService, IXGeometryConverterFactory geomConverterFactory, ILoggerFactory loggerFactory)
        {
            _shapeService = shapeService;
            _geomConverterFactory = geomConverterFactory;
            _loggerFactory = loggerFactory;
        }

        [Fact]
        public void Can_read_and_write_wexbim()
        {
            var geomEngineV6 = _geomConverterFactory.CreateGeometryEngineV6(new MemoryModel(new Ifc4.EntityFactoryIfc4()), _loggerFactory);

            var blockMoq = IfcMoq.IfcBlockMoq() as IIfcCsgPrimitive3D;
            var solid = geomEngineV6.Build(blockMoq) as IXSolid; //initialise the factory with the block
            var meshFactors = geomEngineV6.MeshFactors.SetGranularity(MeshGranularity.Normal);
            IXAxisAlignedBoundingBox bounds;
            byte [] bytes = _shapeService.CreateWexBimMesh(solid, meshFactors, 1000, out bounds);

            

        }
        
    }
}
