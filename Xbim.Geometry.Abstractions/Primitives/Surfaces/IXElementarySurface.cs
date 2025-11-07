namespace Xbim.Geometry.Abstractions
{
    public interface IXElementarySurface : IXSurface
    {
        IXAxis2Placement3d Position { get; }
    }
}