#include "XbimPlane.h"
#include "XbimPoint.h"
#include "XbimDirection.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXPoint^ XbimPlane::Location::get()
			{
				return gcnew XbimPoint(OccHandle()->Location());
			}

			IXDirection^ XbimPlane::Axis::get()
			{
				return gcnew XbimDirection(OccHandle()->Position().Direction());
			}

			IXDirection^ XbimPlane::RefDirection::get()
			{
				return gcnew XbimDirection(OccHandle()->Position().XDirection());
			}
		}
	}
}