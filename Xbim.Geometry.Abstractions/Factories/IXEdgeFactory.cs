using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXEdgeFactory : IXModelScoped
    {
        IXEdge Build(IXPoint start, IXPoint end);

        IXEdge Build(IIfcCurve curve);
        IXEdge Build(IXCurve curve);

    }
}
