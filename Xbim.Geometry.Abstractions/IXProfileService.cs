using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXProfileService
    {
        IXShape Build(IIfcProfileDef profileDef);
    }
}
