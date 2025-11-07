using Xbim.Ifc4.Interfaces;

namespace Xbim.Geometry.Abstractions
{
    public interface IXMaterialFactory
    {
        IXVisualMaterial BuildVisualMaterial(string name, IIfcSurfaceStyleElementSelect styling);

        IXVisualMaterial BuildVisualMaterial(string name);
        
        IXColourRGB BuildColourRGB(double red, double green, double blue);
        IXShapeColour BuildShapeColour(string name, IIfcSurfaceStyleElementSelect surfaceStyle);
    }
}
