using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXSphericalSurface: IXSurface
    {
        double Radius { get; }
        IXAxis2Placement3d Position { get; }
    }
}
