#pragma once
#include "XBoundedSurface.h"
#include "XCurve.h"
#include <Geom_SurfaceOfLinearExtrusion.hxx>

using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref  class  XSurfaceOfLinearExtrusion : XBoundedSurface, IXSurfaceOfLinearExtrusion
			{
			public:
				
				XSurfaceOfLinearExtrusion(Handle(Geom_SurfaceOfLinearExtrusion) surface) :XBoundedSurface(surface)
				{
				}
				property XSurfaceType SurfaceType {virtual XSurfaceType get() override { return XSurfaceType::IfcRectangularTrimmedSurface; }}
				virtual property IXCurve^ BasisCurve {IXCurve^ get() { return XCurve::GeomToXCurve(Handle(Geom_SurfaceOfLinearExtrusion)::DownCast(Ref())->BasisCurve()); }}
				virtual property IXDirection^ Direction { IXDirection^ get() { return gcnew XDirection(Handle(Geom_SurfaceOfLinearExtrusion)::DownCast(Ref())->Direction()); }}
			};
		}
	}
}
