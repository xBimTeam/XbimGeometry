#include "XLine.h"
#include <gp_Lin.hxx>
#include "XPoint.h"
#include "XVector.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXPoint^ XLine::Origin::get()
			{
				return gcnew XPoint(OccLine()->Lin().Location());
			}

			IXVector^ XLine::Direction::get()
			{
				Handle(Geom_LineWithMagnitude) hl = Handle(Geom_LineWithMagnitude)::DownCast(OccHandle());
				if (hl.IsNull())
					return gcnew XVector(OccLine()->Lin().Direction());
				else
					return gcnew XVector(OccLine()->Lin().Direction(), hl->Magnitude());
			}

			
			double XLine::ParametricUnit::get()
			{
				Handle(Geom_LineWithMagnitude) hl = Handle(Geom_LineWithMagnitude)::DownCast(OccLine());
				if (hl.IsNull())
					return 1.0;
				else
					return hl->Magnitude();
			};
		}
	}
}