#pragma once
#include "Unmanaged/NSurfaceFactory.h"
#include "FactoryBase.h"

#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
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
				virtual IXSurface^ Build(IIfcSurface^ surface);
			internal:
				Handle(Geom_Surface) BuildSurface(IIfcSurface^ ifcSurface, XSurfaceType% surfaceType);
				Handle(Geom_Plane) BuildPlane(IIfcPlane^ ifcPlane);
				Handle(Geom_SurfaceOfRevolution) BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcPlane);
				Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion);
				Handle(Geom_Plane) BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane);
				Handle(Geom_Surface) BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface);
				Handle(Geom_RectangularTrimmedSurface) BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface);
				Handle(Geom_Surface) BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots);
				Handle(Geom_Surface) BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots);
				Handle(Geom_Surface) BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface);
				
			protected:
				
			};
		}
	}
}

