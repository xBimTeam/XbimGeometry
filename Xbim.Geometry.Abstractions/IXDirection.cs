namespace Xbim.Geometry.Abstractions
{
    public interface IXDirection
    {
        double X { get; }
        double Y { get; }
        double Z { get; }
        bool Is3d { get; }
    }
}