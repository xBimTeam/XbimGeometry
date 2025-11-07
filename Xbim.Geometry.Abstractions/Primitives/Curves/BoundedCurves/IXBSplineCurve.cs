namespace Xbim.Geometry.Abstractions
{
    public interface IXBSplineCurve : IXBoundedCurve
    {
        XGeometricContinuity Continuity { get; }
        bool IsPeriodic { get; }
        bool IsRational { get; }
    }
}
