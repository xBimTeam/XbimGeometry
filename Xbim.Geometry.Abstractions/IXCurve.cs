using System;

namespace Xbim.Geometry.Abstractions
{
    public interface IXCurve
    {
        XCurveType CurveType { get; }
        bool Is3d { get; }
    }
}
