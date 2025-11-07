namespace Xbim.Geometry.Abstractions
{
    public interface IXCurve
    {
        XCurveType CurveType { get; }
        bool Is3d { get; }
        double Length { get; }
        double FirstParameter { get; }
        double LastParameter { get; }
        IXPoint GetPoint(double uParam);
        IXPoint GetFirstDerivative(double uParam, out IXDirection direction);
        IXPoint GetSecondDerivative(double uParam, out IXDirection direction, out IXDirection normal);
    }
}
