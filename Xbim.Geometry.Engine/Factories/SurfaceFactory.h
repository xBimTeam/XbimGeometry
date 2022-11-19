#pragma once
#include "Unmanaged/NSurfaceFactory.h"
#include "FactoryBase.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class SurfaceFactory : FactoryBase<NSurfaceFactory>, IXSurfaceFactory	
			{					
			public:
				SurfaceFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService,new NSurfaceFactory()) {};
				virtual IXPlane^ BuildPlane(IXPoint^ origin, IXDirection^ normal);
			internal:
				Handle(Geom_Surface) BuildOccSurface(IIfcSurface^ ifcSurface);
				Handle(Geom_Surface) BuildOccPlane(IIfcPlane^ ifcPlane);
				Handle(Geom_Surface) BuildOccSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcPlane);
				Handle(Geom_Surface) BuildOccSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion);
				Handle(Geom_Surface) BuildOccCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane);
				Handle(Geom_Surface) BuildOccRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface);
				Handle(Geom_Surface) BuildOccBSplineSurface(IIfcBSplineSurface^ ifcBSplineSurface);

				Handle(Geom_Surface) BuildOccCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface);
				
			protected:
				
			};
		}
	}
}

