#pragma once
#include "../XbimHandle.h"

#include <Geom_Surface.hxx>

using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref  class  XSurface abstract : XbimHandle<Handle(Geom_Surface)>, IXSurface
			{
			public:
				static IXSurface^ GeomToXSurface(Handle(Geom_Surface));
				XSurface(Handle(Geom_Surface) surface) : XbimHandle(new Handle(Geom_Surface)(surface))
				{
				}
				virtual property XSurfaceType SurfaceType {virtual XSurfaceType get()  abstract; };
				virtual property bool IsUPeriodic {bool get() { return Ref()->IsUPeriodic(); }}
				virtual property bool IsVPeriodic {bool get() { return Ref()->IsVPeriodic(); }}

			};
		}
	}
}