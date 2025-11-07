using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSurfaceService : IXModelScoped
    {
        IXPlane Build(IIfcPlane ifcPlane);
    }
}
