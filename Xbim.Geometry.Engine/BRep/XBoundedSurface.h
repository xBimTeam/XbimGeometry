#pragma once
#include "XSurface.h"

using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref  class  XBoundedSurface abstract : public XSurface, IXBoundedSurface
			{
			public:
				XBoundedSurface(Handle(Geom_Surface) surface) : XSurface(surface)
				{
				}


			};
		}
	}

}
