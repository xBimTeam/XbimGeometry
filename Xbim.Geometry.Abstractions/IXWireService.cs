using System.Threading;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXWireService
    {
        IXWire Build(IIfcCurve curve);
        Task<IXWire> BuildAsync(IIfcCurve curve, CancellationToken cancellationToken);
        Task<IXWire> BuildAsync(IIfcCurve curve);
    }
}
