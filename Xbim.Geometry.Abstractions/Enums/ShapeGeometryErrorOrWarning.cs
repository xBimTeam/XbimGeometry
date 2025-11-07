using System;

namespace Xbim.Geometry.Abstractions
{
    [Flags]
    public enum ShapeGeometryErrorOrWarning : byte
    {
        ErrorBuildFailed = 0,
        NoErrorOrWarnings = 1,
        WarningEmptyShape = 2,
        ErrorProjectionFeaturesFailed = 4,
        ErrorOpeningFeaturesFailed = 8
    }
}
