
#pragma once
#include "XBoundedSurface.h"
#include <Geom_RectangularTrimmedSurface.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref  class  XRectangleTrimmedSurface : public XBoundedSurface, IXRectangleTrimmedSurface
			{
			public:
				XRectangleTrimmedSurface(Handle(Geom_RectangularTrimmedSurface) surface) : XBoundedSurface(surface)
				{
				}
				property XSurfaceType SurfaceType {virtual XSurfaceType get() override { return XSurfaceType::IfcRectangularTrimmedSurface; }}
				virtual void Bounds(double% u1, double% u2, double% v1, double% v2) { 
					double a, b, c, d;
					return Handle(Geom_RectangularTrimmedSurface)::DownCast(Ref())->Bounds(a,b,c,d); 
					u1 = a; u2 = b; v1 = c; v2 = d;
				}
				virtual property IXSurface^ BasisSurface {IXSurface^ get() { return XSurface::GeomToXSurface(Handle(Geom_RectangularTrimmedSurface)::DownCast(Ref())->BasisSurface()); }}

				
			};
		}
	}

}


