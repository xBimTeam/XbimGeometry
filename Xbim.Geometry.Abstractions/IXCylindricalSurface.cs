namespace Xbim.Geometry.Abstractions
{
    public interface IXCylindricalSurface: IXSurface
    {
        double Radius { get; }
        IXAxis2Placement3d Position { get; }
    }
}
