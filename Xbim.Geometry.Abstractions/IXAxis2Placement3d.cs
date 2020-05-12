
namespace Xbim.Geometry.Abstractions
{
    public interface IXAxis2Placement3d: IXAxisPlacement
    {
        IXAxis1Placement Axis { get; }
        IXDirection XDirection { get; }
        IXDirection YDirection { get; }
    }
}
