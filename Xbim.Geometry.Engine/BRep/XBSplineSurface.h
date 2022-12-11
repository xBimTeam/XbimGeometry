#pragma once
#include "XBoundedSurface.h"
#include <Geom_BSplineSurface.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref  class  XBSplineSurface  : public XBoundedSurface, IXBSplineSurface
			{
			public:
				XBSplineSurface(Handle(Geom_BSplineSurface) surface) : XBoundedSurface(surface)
				{
				}
				property XSurfaceType SurfaceType {virtual XSurfaceType get() override { return XSurfaceType::IfcBSplineSurfaceWithKnots; }}
				virtual property int MaxDegree {int get() { return Handle(Geom_BSplineSurface)::DownCast(Ref())->MaxDegree(); }}
				
			};
		}
	}

}

