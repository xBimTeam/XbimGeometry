namespace Xbim.Geometry.Abstractions
{
    public interface IXShape
    {
        XShapeType ShapeType { get; }
        //calculates the bounding box of the shape
        IXAxisAlignedBoundingBox Bounds();
       string Brep { get; }
    }
}
