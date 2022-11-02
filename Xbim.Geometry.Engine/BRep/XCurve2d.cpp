#include "XCurve2d.h"

#include <gp_Pnt.hxx>
#include "X2dPoint.h"
#include "X2dVector.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			const Handle(Geom2d_Curve) XCurve2d::GeomCurve2d(IXCurve^ xCurve)
			{
				return ((XCurve2d^)xCurve)->Ref();
			}

			IXPoint^ XCurve2d::GetPoint(double u)
			{
				gp_Pnt2d pnt;
				OccHandle()->D0(u, pnt);
				return gcnew X2dPoint(pnt);
			}
			IXPoint^ XCurve2d::GetFirstDerivative(double u, IXVector^% normal)
			{
				gp_Pnt2d pnt;
				gp_Vec2d vec;
				OccHandle()->D1(u, pnt, vec);
				normal = gcnew X2dVector(vec);
				return gcnew X2dPoint(pnt);
			}
		}
	}
}