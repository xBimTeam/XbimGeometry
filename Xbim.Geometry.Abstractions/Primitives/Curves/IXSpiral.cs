namespace Xbim.Geometry.Abstractions
{
    public interface IXSpiral : IXCurve
    {
        double? GetCurvature(double uParam);
    }
}