namespace Xbim.Geometry.Abstractions
{
    public interface IXWire: IXShape
    {
        //Length in default model units, use IXGeometryPropertyService to get lengths in a specific imperial or metric unit system
        double Length { get; }
        //Contour Area of the wire NB. This is not the area of the wire as there is no guarantee the wire is closed
        double ContourArea { get; }
        IXEdge[] EdgeLoop { get; }
        
    }
}
