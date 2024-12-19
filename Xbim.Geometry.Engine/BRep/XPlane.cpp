#include "XPlane.h"
#include "XPoint.h"
#include "XDirection.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXPoint^ XPlane::Location::get()
			{
				return gcnew XPoint(Handle(Geom_Plane)::DownCast(Ref())->Location());
			}

			IXDirection^ XPlane::Axis::get()
			{
				return gcnew XDirection(Handle(Geom_Plane)::DownCast(Ref())->Position().Direction());
			}

			IXDirection^ XPlane::RefDirection::get()
			{
				return gcnew XDirection(Handle(Geom_Plane)::DownCast(Ref())->Position().XDirection());
			}
		}
	}
}