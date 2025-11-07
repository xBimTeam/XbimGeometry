using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXBooleanFactory
    {
        IXShape Build(IIfcBooleanResult boolResult);
    }
}
