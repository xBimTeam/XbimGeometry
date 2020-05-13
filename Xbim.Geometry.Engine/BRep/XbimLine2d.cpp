#include "XbimLine2d.h"
#include "../BRep//Xbim2dPoint.h"
#include "../BRep//Xbim2dVector.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			IXPoint^ XbimLine2d::Origin::get()
			{
				gp_Pnt2d pnt = OccHandle()->Location();
				return gcnew Xbim2dPoint(pnt);
			}

			IXVector^ XbimLine2d::Direction::get()
			{
				gp_Dir2d dir = OccHandle()->Direction();
				return gcnew Xbim2dVector(dir, OccHandle()->Magnitude());
			}
			IXPoint^ XbimLine2d::GetPoint(double u)
			{
				gp_Pnt2d pnt = OccHandle()->GetPoint(u);
				return gcnew Xbim2dPoint(pnt);
			}
			IXPoint^ XbimLine2d::GetFirstDerivative(double u, IXVector^% normal)
			{
				gp_Pnt2d pnt;
				gp_Vec2d vec;
				OccHandle()->D1(u , pnt, vec);
				normal = gcnew Xbim2dVector(vec);
				return gcnew Xbim2dPoint(pnt);
			}
		}
	}
}