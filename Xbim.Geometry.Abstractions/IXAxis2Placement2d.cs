
namespace Xbim.Geometry.Abstractions
{
    public interface IXAxis2Placement2d: IXAxisPlacement
    {
        IXPoint Location { get; }
        IXDirection Direction {get;}
    }
}
