namespace Xbim.Geometry.Abstractions
{
    public interface IXAxis2PlacementLinear : IXAxisPlacement
    {
        IXDirection? Axis { get; }
        IXDirection? RefDirection { get; }
        IXPointByDistanceExpression Location { get; }
    }
}