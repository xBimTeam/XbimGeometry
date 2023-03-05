
using FluentAssertions;
using Microsoft.Extensions.Logging;
using System.IO;
using System.Linq;
using Xbim.Common.Geometry;
using Xbim.Common.XbimExtensions;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Tests;
using Xbim.Geometry.WexBim;
using Xbim.Ifc;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
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
        public void Can_read_and_write_block_as_wexbim()
        {
            var geomEngineV6 = _geomConverterFactory.CreateGeometryEngineV6(new MemoryModel(new Ifc4.EntityFactoryIfc4()), _loggerFactory);

            var blockMoq = IfcMoq.IfcBlockMoq() as IIfcCsgPrimitive3D;
            var solid = geomEngineV6.Build(blockMoq) as IXSolid; //initialise the factory with the block
            var meshFactors = geomEngineV6.MeshFactors.SetGranularity(MeshGranularity.Normal);
            
            IXAxisAlignedBoundingBox bounds;
            byte [] bytes = _shapeService.CreateWexBimMesh(solid, meshFactors, 0.001, out bounds);

            var wexBimMesh = new WexBimMesh(bytes);
            wexBimMesh.Vertices.Count().Should().Be(8);
            wexBimMesh.TriangleCount.Should().Be(12);
            wexBimMesh.FaceCount.Should().Be(6);
            
        }

        [Fact]
        public void Can_read_and_write_different_mesh_granularity()
        {
            var geomEngineV6 = _geomConverterFactory.CreateGeometryEngineV6(new MemoryModel(new Ifc4.EntityFactoryIfc4()), _loggerFactory);

            var sphereMoq = IfcMoq.IfcSphereMoq(radius: 10) as IIfcCsgPrimitive3D;
            var solid = geomEngineV6.Build(sphereMoq) as IXSolid; //initialise the factory with the block
            IXAxisAlignedBoundingBox bounds;


            var meshFactors = geomEngineV6.MeshFactors.SetGranularity(MeshGranularity.Fine);
            byte[] bytes = _shapeService.CreateWexBimMesh(solid, meshFactors, 0.001, out bounds);

            var wexBimMesh = new WexBimMesh(bytes);
            wexBimMesh.TriangleCount.Should().Be(1224);
            var fineCount = wexBimMesh.TriangleCount;

            //normal granularity
            meshFactors = geomEngineV6.MeshFactors.SetGranularity(MeshGranularity.Normal);
            bytes = _shapeService.CreateWexBimMesh(solid, meshFactors, 0.001, out bounds);
            wexBimMesh = new WexBimMesh(bytes);
            var normalCount = wexBimMesh.TriangleCount;

            //course granularity
            meshFactors = geomEngineV6.MeshFactors.SetGranularity(MeshGranularity.Course);
            bytes = _shapeService.CreateWexBimMesh(solid, meshFactors, 0.001, out bounds);
            wexBimMesh = new WexBimMesh(bytes);
            var courseCount = wexBimMesh.TriangleCount;
            fineCount.Should().BeGreaterThan(normalCount);
            normalCount.Should().BeGreaterThan(courseCount);
        }

        [Fact]
        public void Can_read_and_write_faceted_brep_to_wexbim_V5_and_V6()
        {
            using var mm = MemoryModel.OpenRead("testfiles/FacetedBrepForSmoothing.ifc");
            var geomEngineV6 = _geomConverterFactory.CreateGeometryEngineV6(mm, _loggerFactory);
            var geomEngineV5 = _geomConverterFactory.CreateGeometryEngineV5(mm, _loggerFactory);
            var ifcFacetedBrep = mm.Instances[32] as IIfcFacetedBrep;
            var facetedBrepV6 = geomEngineV6.Build(ifcFacetedBrep);
            var meshFactors = geomEngineV6.MeshFactors.SetGranularity(MeshGranularity.Fine);
            var bytesV6 = _shapeService.CreateWexBimMesh(facetedBrepV6, meshFactors, 1 / mm.ModelFactors.OneMeter, out IXAxisAlignedBoundingBox bounds);
            var wexBimV6 = new WexBimMesh(bytesV6);
            var facetedBrepV5 = geomEngineV5.Create(ifcFacetedBrep);
            var shapeGeom = geomEngineV5.CreateShapeGeometry(mm.ModelFactors.OneMilliMeter, facetedBrepV5, mm.ModelFactors.Precision, null);
            var wexBimV5 = new WexBimMesh(shapeGeom.ToByteArray());
            wexBimV5.FaceCount.Should().Be(wexBimV6.FaceCount);
            wexBimV5.TriangleCount.Should().Be(wexBimV6.TriangleCount);
            wexBimV5.VertexCount.Should().Be(wexBimV6.VertexCount);
        }
    }
}
