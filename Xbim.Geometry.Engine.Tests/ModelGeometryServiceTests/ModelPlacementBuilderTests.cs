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
            const double precision = 0.0001;;
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
        public void CanGetPlacementWhereCoordinateSystemAdjustedToZero()
        {
            const double precision = 0.0001;
            const int wallId = 275;
            const double wallRelativeCoordX = -206.6877;
            const double wallRelativeCoordY = 105.073;
            const double wallRelativeCoordZ = 0d; 
            
            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var wall = model.Instances[wallId] as IIfcWall;
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var location = builder.BuildLocation(wall.ObjectPlacement, true);

            location.Should().NotBeNull();
            // the location should be adjusted to (0, 0, 0)
            location.Translation.X.Should().BeApproximately(wallRelativeCoordX, precision);
            location.Translation.Y.Should().BeApproximately(wallRelativeCoordY, precision);
            location.Translation.Z.Should().BeApproximately(wallRelativeCoordZ, precision);
        }
        
        [Fact]
        public void CanGetLocationInWorldCoordinateSystem()
        {
            const double precision = 0.0001;
            const int wallId = 275;
            
            var model = MemoryModel.OpenRead("TestFiles/IfcExamples/WCSAdjustmentSample.ifc");
            var wall = model.Instances[wallId] as IIfcWall;
            var engine = _geometryConverterFactory.CreateGeometryEngineV6(model, new LoggerFactory());
            var builder = engine.ModelGeometryService.ModelPlacementBuilder;

            var location = builder.BuildLocation(wall.ObjectPlacement, false);
            
            // move the adjusted location to world coordinates by multiplying it with the WCS matrix
            // after multiplication it should be the same as the global not adjusted location
            var adjustedLocation = builder.BuildLocation(wall.ObjectPlacement, true);
            var movedToWorldCoords = adjustedLocation.Multiplied(builder.WorldCoordinateSystem);
            
            AssertTwoLocationsAreEqual(location, movedToWorldCoords, precision);
        } 
        
        
     
        static void AssertTwoLocationsAreEqual
            (IXLocation loc1, IXLocation loc2, double precision)
        {
            loc1.Translation.X.Should().BeApproximately(loc2.Translation.X, precision);
            loc1.Translation.Y.Should().BeApproximately(loc2.Translation.Y, precision);
            loc1.Translation.Z.Should().BeApproximately(loc2.Translation.Z, precision);
            loc1.M11.Should().BeApproximately(loc2.M11, precision);
            loc1.M12.Should().BeApproximately(loc2.M12, precision);
            loc1.M13.Should().BeApproximately(loc2.M13, precision);
            loc1.M21.Should().BeApproximately(loc2.M21, precision);
            loc1.M22.Should().BeApproximately(loc2.M22, precision);
            loc1.M23.Should().BeApproximately(loc2.M23, precision);
            loc1.M31.Should().BeApproximately(loc2.M31, precision);
            loc1.M32.Should().BeApproximately(loc2.M32, precision);
            loc1.M33.Should().BeApproximately(loc2.M33, precision);
            loc1.ScaleX.Should().BeApproximately(loc2.ScaleX, precision);
            loc1.ScaleY.Should().BeApproximately(loc2.ScaleY, precision);
            loc1.ScaleZ.Should().BeApproximately(loc2.ScaleZ, precision);
        }

    }
}
