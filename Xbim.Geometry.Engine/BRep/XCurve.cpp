#include "XCurve.h"
#include <gp_Pnt.hxx>
#include "XPoint.h"
#include "XVector.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			const Handle(Geom_Curve) XCurve::GeomCurve(IXCurve^ xCurve)
			{
				return ((XCurve^)xCurve)->Ref();
			}

			IXPoint^ XCurve::GetPoint(double u)
			{
				gp_Pnt pnt;
				OccHandle()->D0(u, pnt);
				return gcnew XPoint(pnt);
			}
			IXPoint^ XCurve::GetFirstDerivative(double u, IXVector^% normal)
			{
				gp_Pnt pnt;
				gp_Vec vec;
				OccHandle()->D1(u, pnt, vec);
				normal = gcnew XVector(vec);
				return gcnew XPoint(pnt);
			}
		}
	}
}