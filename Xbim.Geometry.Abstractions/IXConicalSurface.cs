namespace Xbim.Geometry.Abstractions
{
    public interface IXConicalSurface: IXSurface
    {
        double Radius { get; }
        IXAxis2Placement3d Position { get; }
    }
}
