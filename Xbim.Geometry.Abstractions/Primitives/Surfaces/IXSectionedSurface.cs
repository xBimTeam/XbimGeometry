namespace Xbim.Geometry.Abstractions
{
    public interface IXSectionedSurface : IXSurface
    {
        IXCurve Directrix { get; }
    }
}