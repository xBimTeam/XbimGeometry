using Xbim.Ifc4.Interfaces;
using Xbim.Ifc4x3.GeometryResource;

namespace Xbim.Geometry.Abstractions
{
    public interface IXGeometryFactory : IXModelScoped
    {
        IXAxis2Placement2d BuildAxis2Placement2d(IXPoint location, IXVector xAxisDirection);
        IXAxis2Placement3d BuildAxis2Placement3d(IXPoint location, IXVector xAxisDirection, IXVector zAxisDirection);
        IXAxis2Placement2d BuildAxis2Placement2d(IXPoint location, IXDirection xAxisDirection);
        IXAxis2Placement3d BuildAxis2Placement3d(IXPoint location, IXDirection xAisDirection, IXDirection zAxisDirection);
        bool IsFacingAwayFrom(IXFace face, IXDirection direction);
        IXPlane BuildPlane(IIfcPlane plane);
        double Distance(IXPoint a, IXPoint b);
        double IsEqual(IXPoint a, IXPoint b, double tolerance);
        IXDirection NormalAt(IXFace face, IXPoint position, double tolerance);
        IXPoint BuildPoint3d(double x, double y, double z);
        IXPoint BuildPoint3d(IfcPointByDistanceExpression pointByDistanceExpression);
        IXPoint BuildPoint2d(double x, double y);
        IXDirection BuildDirection3d(double x, double y, double z);
        IXDirection BuildDirection2d(double x, double y);
        IXLocation BuildLocation(IIfcObjectPlacement placement);
        IXLocation BuildLocation(IIfcPlacement placement); 
        IXLocation BuildLocation(IIfcAxis2Placement placement); 
        IXLocation BuildLocation(IfcAxis2PlacementLinear linearPlacement); 
        IXMatrix BuildTransform(IIfcCartesianTransformationOperator transformOp);
        void BuildMapTransform(IIfcCartesianTransformationOperator transform, IIfcAxis2Placement origin, out IXLocation location, out IXMatrix matrix);
    }
}
