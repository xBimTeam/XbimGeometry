#include "XLine2d.h"
#include "X2dPoint.h"
#include "X2dVector.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			IXPoint^ XLine2d::Origin::get()
			{
				gp_Pnt2d pnt = OccLine2d()->Location();
				return gcnew X2dPoint(pnt);
			}

			IXVector^ XLine2d::Direction::get()
			{
				gp_Dir2d dir = OccLine2d()->Direction();
				return gcnew X2dVector(dir, OccLine2d()->Magnitude());
			}
			
		}
	}
}