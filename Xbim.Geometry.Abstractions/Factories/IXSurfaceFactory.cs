using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSurfaceFactory : IXModelScoped
    {
        IXSurface Build(IIfcSurface surface);
        IXPlane BuildPlane(IXPoint origin, IXDirection normal);
    }
}
