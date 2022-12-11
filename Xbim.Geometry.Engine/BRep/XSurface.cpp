#include "XSurface.h"
#include <Geom_BSplineSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_SphericalSurface.hxx>

#include "XPlane.h"
#include "XCylindricalSurface.h"
#include "XSphericalSurface.h"
#include "XBSplineSurface.h"
#include "XRectangleTrimmedSurface.h"
#include "XSurfaceOfRevolution.h"
#include "XSurfaceOfLinearExtrusion.h"


namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXSurface^ XSurface::GeomToXSurface(Handle(Geom_Surface) surface)
			{
				auto plane = Handle(Geom_Plane)::DownCast(surface);
				if (!plane.IsNull()) return gcnew XPlane(plane);

				auto bspline = Handle(Geom_BSplineSurface)::DownCast(surface);
				if (!bspline.IsNull()) return gcnew XBSplineSurface(bspline);

				auto rectSurface = Handle(Geom_RectangularTrimmedSurface)::DownCast(surface);
				if (!rectSurface.IsNull()) return gcnew XRectangleTrimmedSurface(rectSurface);

				auto revSurface = Handle(Geom_SurfaceOfRevolution)::DownCast(surface);
				if (!revSurface.IsNull()) return gcnew XSurfaceOfRevolution(revSurface);

				auto linSurface = Handle(Geom_SurfaceOfLinearExtrusion)::DownCast(surface);
				if (!linSurface.IsNull()) return gcnew XSurfaceOfLinearExtrusion(linSurface);

				auto cylSurface = Handle(Geom_CylindricalSurface)::DownCast(surface);
				if (!cylSurface.IsNull()) return gcnew XCylindricalSurface(cylSurface);

				auto sphereSurface = Handle(Geom_SphericalSurface)::DownCast(surface);
				if (!sphereSurface.IsNull()) return gcnew XSphericalSurface(sphereSurface);

				throw gcnew System::NotImplementedException("Surface not implemented yet");
				
				/*TODO
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcCurveBoundedPlane:
					break;
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcCurveBoundedSurface:
					break;

				case Xbim::Geometry::Abstractions::XSurfaceType::IfcToroidalSurface:
					break;
				default:
					break;
				}
				switch*/
			}
		}
	}
}