namespace Xbim.Geometry.Abstractions
{
    public interface IXSurface
    {
        XSurfaceType SurfaceType { get; }
        bool IsUPeriodic { get; }
        bool IsVPeriodic { get; }
    }
}
