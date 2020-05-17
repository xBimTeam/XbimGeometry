using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXCurveService
    {
        IXCurve Build(IIfcCurve curve);
    }
}
