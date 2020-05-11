using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXAxis2Placement
    {
        IXAxis1Placement Axis { get; }
        IXDirection XDirection { get; }
        IXDirection YDirection { get; }
    }
}
