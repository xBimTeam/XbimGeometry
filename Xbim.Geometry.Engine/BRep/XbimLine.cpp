#include "XbimLine.h"
#include <gp_Lin.hxx>
#include "XbimPoint.h"
#include "XbimVector.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXPoint^ XbimLine::Origin::get()
			{			
				return gcnew XbimPoint((*Ptr())->Lin().Location());
			}

			IXVector^ XbimLine::Direction::get()
			{
				
				return gcnew XbimVector(OccHandle()->Lin().Direction(), parametricUnit);
			}

			IXPoint^ XbimLine::GetPoint(double u)
			{
				gp_Pnt pnt;
				OccHandle()->D0(u * ParametricUnit, pnt);
				return gcnew XbimPoint(pnt.X(), pnt.Y(), pnt.Z());
			}
			IXPoint^ XbimLine::GetFirstDerivative(double u, IXVector^ %normal)
			{
				gp_Pnt pnt;
				gp_Vec vec;
				OccHandle()->D1(u * ParametricUnit, pnt, vec);
				normal = gcnew XbimVector(vec);
				return gcnew XbimPoint(pnt.X(), pnt.Y(), pnt.Z());
			}
		}
	}
}