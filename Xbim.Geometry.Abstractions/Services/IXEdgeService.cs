namespace Xbim.Geometry.Abstractions
{
    public interface IXEdgeService : IXModelScoped
    {
        IXEdge Build(IXCurve curve);
    }
}
