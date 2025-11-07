namespace Xbim.Geometry.Abstractions
{
    /// <summary>
    /// A 2 or 3 dimensional point
    /// </summary>
    public interface IXPoint
    {
        bool Is3d { get; }
        double X { get; }
        double Y { get; }
        double Z { get; }
    }
}