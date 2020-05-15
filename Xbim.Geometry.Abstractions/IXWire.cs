using System.Collections.Generic;

namespace Xbim.Geometry.Abstractions
{
    public interface IXWire: IXShape
    {
        IEnumerable<IXEdge> EdgeLoop { get; }
    }
}
