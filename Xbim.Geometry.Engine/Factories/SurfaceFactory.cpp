#include "SurfaceFactory.h"
#include <Geom_Plane.hxx>


#include "../BRep//XPlane.h"
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXPlane^ SurfaceFactory::BuildPlane(IXPoint^ origin, IXDirection^ normal)
			{
				
				Handle(Geom_Plane) occPlane = Ptr()->BuildPlane(origin->X, origin->Y, origin->Z, normal->X, normal->Y, normal -> Z);
				if (occPlane.IsNull())
					throw gcnew Exception("Invalid arguments for plane");
				else
					return gcnew XPlane(occPlane);

			}
		}
	}
}
