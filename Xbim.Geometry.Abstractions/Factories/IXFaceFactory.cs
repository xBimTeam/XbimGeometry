

namespace Xbim.Geometry.Abstractions
{
    public interface IXFaceFactory:IXModelScoped
    {
        IXFace BuildFace(IXSurface surface, IXWire[] wires);
    }
}
