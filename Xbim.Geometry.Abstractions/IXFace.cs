using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXFace: IXShape
    {
        double Tolerance { get; }
        IXWire OuterBound { get; }
        IEnumerable<IXWire> InnerBounds { get; }
        IXSurface Surface { get; }
    }
}
