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
				SurfaceFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService,new NSurfaceFactory()) {};
				virtual IXPlane^ BuildPlane(IXPoint^ origin, IXDirection^ normal);
			internal:
				Handle(Geom_Surface) BuildSurface(IIfcSurface^ ifcSurface);
				Handle(Geom_Plane) BuildPlane(IIfcPlane^ ifcPlane);
				Handle(Geom_Surface) BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcPlane);
				Handle(Geom_Surface) BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion);
				Handle(Geom_Plane) BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane);
				Handle(Geom_Surface) BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface);
				Handle(Geom_Surface) BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface);
				Handle(Geom_Surface) BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots);
				Handle(Geom_Surface) BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots);
				Handle(Geom_Surface) BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface);
				
			protected:
				
			};
		}
	}
}

