using System.Threading;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSolidService
    {
        IXSolid Build(IIfcSolidModel ifcSolid);
        Task<IXSolid> BuildAsync(IIfcSolidModel ifcSolid);
        Task<IXSolid> BuildAsync(IIfcSolidModel ifcSolid, CancellationToken token);

        IXSolid Build(IIfcCsgPrimitive3D ifcCsgPrimitive);
        Task<IXSolid> BuildAsync(IIfcCsgPrimitive3D ifcCsgPrimitive);
        Task<IXSolid> BuildAsync(IIfcCsgPrimitive3D ifcCsgPrimitive, CancellationToken token);


        IXSolid Build(IIfcBooleanOperand boolOperand);
        Task<IXSolid> BuildAsync(IIfcBooleanOperand boolOperand);
        Task<IXSolid> BuildAsync(IIfcBooleanOperand boolOperand, CancellationToken token);
    }
}
