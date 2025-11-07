namespace Xbim.Geometry.Abstractions
{
    public interface IXVertex : IXShape
    {
        double Tolerance { get; }
        IXPoint VertexGeometry { get; }
    }
}
