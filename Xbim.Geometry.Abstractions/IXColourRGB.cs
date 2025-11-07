using System;
using System.Collections.Generic;
using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.GeometryResource;

namespace Xbim.Geometry.Abstractions
{
    public interface IXColourRGB : IEqualityComparer<IXColourRGB>, IEquatable<IXColourRGB>
    {
        double Red { get; }
        double Green { get;  }
        double Blue { get; }
    }
}
