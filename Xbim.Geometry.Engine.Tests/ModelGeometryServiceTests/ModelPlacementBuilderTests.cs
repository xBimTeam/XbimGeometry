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
            // no placements tree could be deduced, default WCS is origin
            builder.WorldCoordinateSystem.X.Should().Be(0);
            builder.WorldCoordinateSystem.Y.Should().Be(0);
            builder.WorldCoordinateSystem.Z.Should().Be(0);
        }

        [Fact]
        public void CanGetModelWorldOrigin()
        {
            const double precision = 0.001;
            ;
            const double originX = 100d;
            const double originY = 300d;

            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());

            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var modelWorldOrigin = builder.WorldCoordinateSystem;

            modelWorldOrigin.X.Should().BeApproximately(originX, precision);
            modelWorldOrigin.Y.Should().BeApproximately(originY, precision);
        }

        [Fact]
        public void CanGetPlacementWhereCoordinateSystemAdjustedToOrigin()
        {
            const double precision = 0.001;
            const int wallId = 275;
            const double wallRelativeCoordX = -220.448;
            const double wallRelativeCoordY = -71.852;
            // these aligned values are what you get from xbim xplorer
            const double wallRelativeCoordXAxisAligned = -206.6877;
            const double wallRelativeCoordYAxisAligned = 105.073;
            
            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var wall = model.Instances[wallId] as IIfcWall;
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var location = builder.BuildLocation(wall.ObjectPlacement, true);

            location.Should().NotBeNull();
            
            location.Translation.X.Should().BeApproximately(wallRelativeCoordX, precision);
            location.Translation.Y.Should().BeApproximately(wallRelativeCoordY, precision);
            
            // Multiplying by the inverse root placement should negate any rotation
            // and align the placement with the canonical basis
            // this is just for asserting the location is correct
            var inverseRoot = builder.RootPlacement.Inverted();
            var adjustedLocation = location.Multiplied(inverseRoot);
            adjustedLocation.Translation.X.Should().BeApproximately(wallRelativeCoordXAxisAligned, precision);
            adjustedLocation.Translation.Y.Should().BeApproximately(wallRelativeCoordYAxisAligned, precision);
            adjustedLocation.Translation.Z.Should().BeApproximately(0d, precision);
        }

        [Fact]
        public void CanGetLocationInWorldCoordinateSystem()
        {
            const double precision = 0.001;
            const int wallId = 275;
            const double wallRelativeCoordX = -220.448;
            const double wallRelativeCoordY = -71.852;
            const double wallRelativeCoordZ = 0d;

            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var wall = model.Instances[wallId] as IIfcWall;
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var location = builder.BuildLocation(wall.ObjectPlacement, false);

            location.Should().NotBeNull();
            // adding the WCS to the relative coordinates
            // should add up to the world coordinates of the wall
            location.Translation.X.Should().BeApproximately
                (wallRelativeCoordX + builder.WorldCoordinateSystem.X, precision);
            location.Translation.Y.Should().BeApproximately
                (wallRelativeCoordY + builder.WorldCoordinateSystem.Y, precision);
            location.Translation.Z.Should().BeApproximately
                (wallRelativeCoordZ + builder.WorldCoordinateSystem.Z, precision);
        }
    }
}