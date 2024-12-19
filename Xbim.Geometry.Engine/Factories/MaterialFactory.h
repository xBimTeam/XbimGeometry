#pragma once
#include "FactoryBase.h"
#include "./Unmanaged//NMaterialFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class MaterialFactory : public FactoryBase<NMaterialFactory>, IXMaterialFactory
			{
			public:
				MaterialFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NMaterialFactory()) {}

				virtual Xbim::Geometry::Abstractions::IXVisualMaterial^ BuildVisualMaterial(System::String^ name, Xbim::Ifc4::Interfaces::IIfcSurfaceStyleElementSelect^ styling);

				virtual Xbim::Geometry::Abstractions::IXVisualMaterial^ BuildVisualMaterial(System::String^ name);
				// Inherited via IXGeometryProcedures
				virtual Xbim::Geometry::Abstractions::IXColourRGB^ BuildColourRGB(double red, double green, double blue);
				virtual Xbim::Geometry::Abstractions::IXShapeColour^ BuildShapeColour(System::String^ name, IIfcSurfaceStyleElementSelect^ surfaceStyle);
			};
		}
	}
}

