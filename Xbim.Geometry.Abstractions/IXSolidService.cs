using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSolidService
    {
        IXSolid Build(IIfcSolidModel ifcSolid);
        IXSolid Build(IIfcCsgPrimitive3D ifcCsgPrimitive);
        IXSolid Build(IIfcBooleanOperand boolOperand);
    }
}
