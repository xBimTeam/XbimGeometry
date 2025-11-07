namespace Xbim.Geometry.Abstractions
{
    public interface IXFace : IXShape
    {
        //Area in default model units, use IXGeometryPropertyService to get areas in a specific imperial or metric unit system
        double Area { get; }
        double Tolerance { get; }
        IXWire OuterBound { get; }
        IXWire[] InnerBounds { get; }
        IXSurface Surface { get; }

    }
}
