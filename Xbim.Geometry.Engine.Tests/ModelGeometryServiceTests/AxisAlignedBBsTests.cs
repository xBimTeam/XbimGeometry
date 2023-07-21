using FluentAssertions;
using Microsoft.Extensions.Logging;
using Xbim.Geometry.Abstractions;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4;
using Xbim.IO.Memory;
using Xunit;

namespace Xbim.Geometry.Engine.Tests
{
    public class AxisAlignedBBsTests
    {
        private readonly MemoryModel _dummyModel = new MemoryModel(new EntityFactoryIfc4());
        private readonly IXModelGeometryService _modelSvc;
        private readonly IXGeometryPrimitives _geometryPrimitivesFactory;
        private const double TOLERANCE = 1e-5;

        public AxisAlignedBBsTests(IXbimGeometryServicesFactory geometryServicesFactory,
                                IXGeometryPrimitives geometryPrimitivesFactory,
                                ILoggerFactory loggerFactory)
        {
            _modelSvc = geometryServicesFactory.CreateModelGeometryService(_dummyModel, loggerFactory);
            _geometryPrimitivesFactory = geometryPrimitivesFactory;
        }

        [Fact]
        public void CanTransformBoundingBox()
        {
            var solidService = _modelSvc.SolidFactory;
            var ifcSweptDisk = IfcMoq.IfcSweptDiskSolidMoq
                (startParam: 0, endParam: 100, radius: 30, innerRadius: 15);
            var solid = (IXSolid)solidService.Build(ifcSweptDisk);
            var bb = solid.Bounds();

            foreach (var location in Locations)
            {
                var transformed = bb.Transformed(location);

                transformed.Should().NotBeNull();
                transformed.Centroid.X.Should().BeApproximately((bb.Centroid.X * location.Scale) + location.OffsetX, TOLERANCE);
                transformed.Centroid.Y.Should().BeApproximately((bb.Centroid.Y * location.Scale) + location.OffsetY, TOLERANCE);
                transformed.Centroid.Z.Should().BeApproximately((bb.Centroid.Z * location.Scale) + location.OffsetZ, TOLERANCE);
                transformed.LenX.Should().BeApproximately(bb.LenX * location.Scale, TOLERANCE);
                transformed.LenY.Should().BeApproximately(bb.LenY * location.Scale, TOLERANCE);
                transformed.LenZ.Should().BeApproximately(bb.LenZ * location.Scale, TOLERANCE);
            }
        }


        #region Helpers

        public IEnumerable<IXLocation> Locations
        {
            get
            {
                var result = new List<IXLocation>()
                {
                    Location(2, 2, 2, 1, 0, 0, 0, 1), // translation
                    Location(0, 0, 0, 0.5, 0, 0, 0, 1), // scale down
                };

                return result;
            }
        }


        private IXLocation Location
            (double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz)
        {
            var location = _geometryPrimitivesFactory
                .BuildLocation(tx, ty, tz, sc, qw, qx, qy, qz);
            return location;
        }

        #endregion
    }

}

