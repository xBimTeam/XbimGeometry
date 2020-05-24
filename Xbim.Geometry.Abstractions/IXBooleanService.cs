using System.Threading;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXBooleanService
    {
        IXShape Build(IIfcBooleanResult boolResult);
        Task<IXShape> BuildAsync(IIfcBooleanResult boolResult);
        Task<IXShape> BuildAsync(IIfcBooleanResult boolResult, CancellationToken token);
    }
}
