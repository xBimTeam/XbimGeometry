
namespace Xbim.Geometry.Abstractions
{
    public interface IXAxis1Placement : IXAxisPlacement
    {
        IXPoint Location { get; }
        IXDirection Direction {get;}
    }
}
