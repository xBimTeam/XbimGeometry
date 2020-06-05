namespace Xbim.Geometry.Abstractions
{
    public interface IXCompositeCurve : IXBoundedCurve
    {
        int NumberOfSegments { get; }
    }
}
