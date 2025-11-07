
namespace Xbim.Geometry.Abstractions
{
    public enum XCurveType
    {
        IfcCurve, //specific type unknown
        IfcBSplineCurveWithKnots,
        IfcCircle,
        IfcCompositeCurve,
        IfcCompositeCurveOnSurface,
        IfcEllipse,
        IfcIndexedPolyCurve,
        IfcLine,
        IfcOffsetCurve3D,
        IfcOffsetCurve2D,
        IfcPcurve,
        IfcPolyline,
        IfcRationalBSplineCurveWithKnots,
        IfcSurfaceCurve,
        IfcTrimmedCurve,
        
        // IFC4x3
        IfcSineSpiral,
        IfcCosineSpiral,
        IfcClothoid,
        IfcSecondOrderPolynomialSpiral,
        IfcThirdOrderPolynomialSpiral,
        IfcSeventhOrderPolynomialSpiral,
        
        IfcPolynomialCurve,
        
        IfcGradientCurve,
        IfcSegmentedReferenceCurve
        
    };

}
