

namespace Xbim.Geometry.Abstractions
{
    public interface IXCircle : IXCurve
    {
        double Radius { get; }
        IXAxis2Placement Position { get; }
    }
}
