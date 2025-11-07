using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXProfileFactory : IXModelScoped
    {
        IXFace BuildFace(IIfcProfileDef profileDef);
        IXWire BuildWire(IIfcProfileDef profileDef);
        IXEdge BuildEdge(IIfcProfileDef profileDef);
        IXCurve BuildCurve(IIfcProfileDef profileDef);
    }
}
