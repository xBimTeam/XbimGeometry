namespace Xbim.Geometry.Abstractions
{
    public enum XProfileDefType
    {
        IfcArbitraryClosedProfileDef,
        IfcArbitraryProfileDefWithVoids,
        IfcArbitraryOpenProfileDef,
        IfcCenterLineProfileDef,
        IfcCompositeProfileDef,
        IfcDerivedProfileDef,
        IfcMirroredProfileDef,
        IfcAsymmetricIShapeProfileDef,
        IfcCShapeProfileDef,
        IfcCircleProfileDef,
        IfcCircleHollowProfileDef,
        IfcEllipseProfileDef,
        IfcIShapeProfileDef,
        IfcLShapeProfileDef,
        IfcRectangleProfileDef,
        IfcRectangleHollowProfileDef,
        IfcRoundedRectangleProfileDef,
        IfcTShapeProfileDef,
        IfcTrapeziumProfileDef, // deprecated in 4x3
        IfcUShapeProfileDef,
        IfcZShapeProfileDef,
        
        IfcOpenCrossProfileDef // new in 4x3
    }
}
