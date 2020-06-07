namespace Xbim.Geometry.Abstractions
{
    public interface IXBSplineCurve : IXBoundedCurve
    {
        IXGeometricContinuity Continuity { get; }
    }
}
