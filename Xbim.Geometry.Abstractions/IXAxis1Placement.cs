using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXAxis1Placement
    {
        IXPoint Location { get; }
        IXDirection Direction {get;}
    }
}
