namespace Xbim.Geometry.Abstractions
{
    public interface IXBoundedCurve : IXCurve
    {
        IXPoint StartPoint { get; }
        IXPoint EndPoint { get; }
    }
}
