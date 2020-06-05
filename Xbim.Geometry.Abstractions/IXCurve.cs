using System;

namespace Xbim.Geometry.Abstractions
{
    public interface IXCurve
    {
        XCurveType CurveType { get; }
        bool Is3d { get; }
        double Length { get; }
        double FirstParameter { get; }
        double LastParameter { get; }
    }
}
