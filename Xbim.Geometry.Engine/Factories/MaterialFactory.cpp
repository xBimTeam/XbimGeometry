#include "MaterialFactory.h"
#include "../Visual/VisualMaterial.h"
#include "../Visual/XbimShapeColour.h"
#include "../Visual/ColourRGB.h"

Xbim::Geometry::Abstractions::IXVisualMaterial^ Xbim::Geometry::Factories::MaterialFactory::BuildVisualMaterial(System::String^ name, Xbim::Ifc4::Interfaces::IIfcSurfaceStyleElementSelect^ styling)
{
    return gcnew Xbim::Geometry::Visual::VisualMaterial(name,styling);
}

Xbim::Geometry::Abstractions::IXVisualMaterial^ Xbim::Geometry::Factories::MaterialFactory::BuildVisualMaterial(System::String^ name)
{
    return gcnew Xbim::Geometry::Visual::VisualMaterial(name);
}

Xbim::Geometry::Abstractions::IXColourRGB^ Xbim::Geometry::Factories::MaterialFactory::BuildColourRGB(double red, double green, double blue)
{
    return gcnew Xbim::Geometry::Visual::ColourRGB(red,green,blue);
}

Xbim::Geometry::Abstractions::IXShapeColour^ Xbim::Geometry::Factories::MaterialFactory::BuildShapeColour(System::String^ name, IIfcSurfaceStyleElementSelect^ surfaceStyle)
{
    return gcnew Xbim::Geometry::Visual::XbimShapeColour(name,surfaceStyle);
}
