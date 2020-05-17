using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXBooleanService
    {
        IXShape Build(IIfcBooleanResult boolResult);
    }
}
