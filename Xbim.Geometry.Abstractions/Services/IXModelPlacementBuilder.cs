using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXModelPlacementBuilder
    {
        IXLocation BuildLocation(IIfcObjectPlacement placement, bool adjustWcs);
        public IXPoint WorldCoordinateSystem { get; }
        public IXLocation RootPlacement { get; }
    }
}
