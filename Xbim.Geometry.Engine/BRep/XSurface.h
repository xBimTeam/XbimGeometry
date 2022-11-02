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
				XSurface(Handle(Geom_Surface) surface) : XbimHandle(new Handle(Geom_Surface)(surface))
				{
				}
				property XSurfaceType SurfaceType {virtual XSurfaceType get()  abstract; }
			};
		}
	}
}