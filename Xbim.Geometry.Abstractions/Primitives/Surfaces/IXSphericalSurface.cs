namespace Xbim.Geometry.Abstractions
{
    public interface IXSphericalSurface : IXSurface
    {
        double Radius { get; }
        IXAxis2Placement3d Position { get; }
    }
}
