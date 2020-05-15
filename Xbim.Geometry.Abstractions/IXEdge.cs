using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXEdge: IXShape
    {
        double Tolerance { get; }
        IXCurve EdgeGeometry { get; }
        IXVertex EdgeStart { get; }
        IXVertex EdgeEnd { get; }
    }
}
