namespace Xbim.Geometry.Abstractions
{
    public interface IXPoint
    {
        /// <summary>
        /// True if the point is a 3d point
        /// </summary>
        bool Is3d { get; }
        double X { get; }
        double Y { get; }
        double Z { get; }
    }
}