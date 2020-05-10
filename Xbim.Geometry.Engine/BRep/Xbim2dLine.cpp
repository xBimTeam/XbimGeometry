#include "Xbim2dLine.h"
#include "../BRep//Xbim2dPoint.h"
#include "../BRep//Xbim2dVector.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			IXPoint^ Xbim2dLine::Origin::get()
			{
				gp_Pnt2d pnt = OccHandle()->Location();
				return gcnew Xbim2dPoint(pnt);
			}

			IXVector^ Xbim2dLine::Direction::get()
			{
				gp_Dir2d dir = OccHandle()->Direction();
				return gcnew Xbim2dVector(dir, parametricUnit);
			}
			IXPoint^ Xbim2dLine::GetPoint(double u)
			{
				gp_Pnt2d pnt;
				OccHandle()->D0(u * ParametricUnit, pnt);
				return gcnew Xbim2dPoint(pnt);
			}
			IXPoint^ Xbim2dLine::GetFirstDerivative(double u, IXVector^% normal)
			{
				gp_Pnt2d pnt;
				gp_Vec2d vec;
				OccHandle()->D1(u * ParametricUnit, pnt, vec);
				normal = gcnew Xbim2dVector(vec);
				return gcnew Xbim2dPoint(pnt);
			}
		}
	}
}