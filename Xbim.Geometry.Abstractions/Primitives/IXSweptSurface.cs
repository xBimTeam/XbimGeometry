namespace Xbim.Geometry.Abstractions
{
    public interface IXSweptSurface
    {
        IXCurve BasisCurve { get; }
        IXDirection Direction { get; }

    }
}
