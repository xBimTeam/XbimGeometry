#include "XCurve2d.h"

#include <gp_Pnt.hxx>
#include "X2dPoint.h"
#include "X2dVector.h"
#include "XDirection.h"
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
				auto dir  = gcnew XDirection(vec.X(), vec.Y());
				direction = dir;
				return gcnew X2dPoint(pnt);
			}
			IXPoint^ XCurve2d::GetSecondDerivative(double u, IXDirection^% direction, IXDirection^% normal)
			{
				gp_Pnt2d pnt;
				try
				{

					gp_Vec2d dir2d, norm2d;
					OccHandle()->D2(u, pnt, dir2d, norm2d);				
					auto dir = gcnew XDirection(dir2d.X(),dir2d.Y());
					direction = dir;
					auto norm = gcnew XDirection(norm2d.X(), norm2d.Y());
					normal = norm;
					//normal can be null				
				}
				catch (const Standard_ConstructionError& ) //catch if we break the OCC rule that a direction cannot have all coords as 0
				{
					auto norm  = gcnew XDirection();
					normal = norm;
				}
				
				return gcnew X2dPoint(pnt);
				
			}
		}
	}
}