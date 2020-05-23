using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXWireService
    {
        IXWire Build(IIfcCurve curve);
    }
}
