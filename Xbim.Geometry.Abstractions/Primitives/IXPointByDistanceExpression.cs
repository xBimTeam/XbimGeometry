namespace Xbim.Geometry.Abstractions
{
    public interface IXPointByDistanceExpression
    {
        double DistanceAlong { get; }
        XCurveMeasureType MeasureType { get; }
        double? OffsetLateral { get; }
        double? OffsetVertical { get; }
        double? OffsetLongitudinal { get; }
        IXCurve BasisCurve { get; }
    }
}