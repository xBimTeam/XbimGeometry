namespace Xbim.Geometry.Abstractions
{
    public interface IXTrimmedCurve : IXBoundedCurve
    {
        IXCurve BasisCurve { get; }
    }
}
