using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.GeometryResource;

namespace Xbim.Geometry.Abstractions
{
    public interface IXCurveFactory : IXModelScoped
    {
        IXCurve Build(IIfcCurve curve);
        IXCurve BuildDirectrix(IIfcCurve curve, double? startParam, double? endParam);
        IXCurve BuildSpiral(IfcSpiral curve, double startParam, double endParam);
        IXCurve BuildPolynomialCurve2d(IfcPolynomialCurve curve, double startParam, double endParam);
    }
}
