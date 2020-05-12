using System;
using System.Collections.Generic;
using System.Text;

namespace Xbim.Geometry.Abstractions
{
    public interface IXEllipse: IXConic
    {
        double MajorRadius { get; }
        double MinorRadius { get; }
    }
}
