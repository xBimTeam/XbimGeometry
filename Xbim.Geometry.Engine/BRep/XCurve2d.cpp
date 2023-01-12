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
			IXPoint^ XCurve2d::GetFirstDerivative(double u, IXDirection^% direction)
			{
				gp_Pnt2d pnt;
				gp_Vec2d vec;
				OccHandle()->D1(u, pnt, vec);
				direction = gcnew XDirection(vec.X(), vec.Y());
				return gcnew X2dPoint(pnt);
			}
			IXPoint^ XCurve2d::GetSecondDerivative(double u, IXDirection^% direction, IXDirection^% normal)
			{
				gp_Pnt2d pnt;
				gp_Vec2d vec, norm;
				OccHandle()->D2(u, pnt, vec, norm);
				direction = gcnew XDirection(vec.X(), vec.Y());
				normal = gcnew XDirection(norm.X(), norm.Y());
				return gcnew X2dPoint(pnt);
			}
		}
	}
}