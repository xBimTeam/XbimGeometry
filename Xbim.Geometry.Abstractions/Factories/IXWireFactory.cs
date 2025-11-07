using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXWireFactory : IXModelScoped
    {
        IXWire BuildWire(IXPoint[]  points);
               
               
        IXWire Build(IIfcCurve ifcCurve);
        IXWire Build(IIfcProfileDef ifcProfileDef);
    }
}
