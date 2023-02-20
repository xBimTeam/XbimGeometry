#pragma once
#include "Unmanaged/NSurfaceFactory.h"
#include "FactoryBase.h"

#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
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
				Handle(Geom_SurfaceOfRevolution) BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcSurfaceOfRevolution);
				Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion);
				TopoDS_Face BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane);
				TopoDS_Face BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface);
				Handle(Geom_RectangularTrimmedSurface) BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface);
				Handle(Geom_BSplineSurface) BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots);
				Handle(Geom_BSplineSurface) BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots);
				Handle(Geom_CylindricalSurface) BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface);
				Handle(Geom_SphericalSurface) BuildSphericalSurface(IIfcSphericalSurface^ ifcSphericalSurface);
				Handle(Geom_ToroidalSurface) BuildToroidalSurface(IIfcToroidalSurface^ ifcToroidalSurface);
			protected:
				
			};
		}
	}
}

