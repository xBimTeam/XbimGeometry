using FluentAssertions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xbim.Geometry.Abstractions;
using Xbim.Ifc4.Interfaces;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests.ModelGeometryServiceTests
{
    public class ModelPlacementBuilderTests
    {
        private readonly IXGeometryConverterFactory _geometryConverterFactory;

        public ModelPlacementBuilderTests(IXGeometryConverterFactory geometryConverterFactory)
        {
            _geometryConverterFactory = geometryConverterFactory;
        }

        [Fact]
        public void CanCreateModelPlacementBuilder()
        {
            using var model = new MemoryModel(new Ifc2x3.EntityFactoryIfc2x3());
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            builder.Should().NotBeNull();
            // no placements tree could be deduced, default WCS is identity
            builder.WorldCoordinateSystem.IsIdentity.Should().BeTrue(); 
        }
        
        [Fact]
        public void CanModelWorldOrigin()
        {
            const double precision = 1e5;
            const double originX = 100d;
            const double originY = 300d;
            
            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var modelWorldOrigin = builder.WorldCoordinateSystem.Translation;

            modelWorldOrigin.X.Should().BeApproximately(originX, precision); 
            modelWorldOrigin.Y.Should().BeApproximately(originY, precision); 
        }
        
        [Fact]
        public void CanGetLocationInWorldCoordinateSystem()
        {
            const double precision = 1e5;
            const int wallId = 275;
            const double wallRelativeCoordX = -206.6877;
            const double wallRelativeCoordY = 105.073;
            const double originX = 100d;
            const double originY = 300d;
            
            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var wall = model.Instances[wallId] as IIfcWall;
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var location = builder.BuildLocation(wall.ObjectPlacement, false);
            location.Should().NotBeNull();
            location.Translation.X.Should().BeApproximately(wallRelativeCoordX + originX, precision);
            location.Translation.Y.Should().BeApproximately(wallRelativeCoordY + originY, precision);
        }
        
        [Fact]
        public void CanGetPlacementWhereCoordinateSystemAdjustedToZero()
        {
            const double precision = 1e5;
            const int wallId = 275;
            const double wallRelativeCoordX = -206.6877;
            const double wallRelativeCoordY = 105.073;

            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var wall = model.Instances[wallId] as IIfcWall;
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var location = builder.BuildLocation(wall.ObjectPlacement, true);
            location.Should().NotBeNull();
            location.Translation.X.Should().BeApproximately(wallRelativeCoordX, precision);
            location.Translation.Y.Should().BeApproximately(wallRelativeCoordY, precision);
        }
        
    }
}
