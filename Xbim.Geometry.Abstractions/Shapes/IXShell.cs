namespace Xbim.Geometry.Abstractions
{
    public interface IXShell : IXShape
    {
        //Surface Area in default model units, use IXGeometryPropertyService to get surface areas in a specific imperial or metric unit system
        double SurfaceArea { get; }
        IXFace[] Faces { get; }
    }
}
