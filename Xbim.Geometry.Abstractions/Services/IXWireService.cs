using System.Threading;
using System.Threading.Tasks;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXWireService : IXModelScoped
    {
        IXWire Build(IIfcCurve curve);
        Task<IXWire> BuildAsync(IIfcCurve curve, CancellationToken cancellationToken);
        Task<IXWire> BuildAsync(IIfcCurve curve);

        IXWire Build(IIfcProfileDef def);
        Task<IXWire> BuildAsync(IIfcProfileDef def, CancellationToken cancellationToken);
        Task<IXWire> BuildAsync(IIfcProfileDef def);
    }
}
