#include "XCurve.h"
#include "XLine.h"
#include "XCircle.h"
#include "XEllipse.h"
#include "XBSplineCurve.h"
#include<Geom_Line.hxx>

#include<Geom_Circle.hxx>
#include<Geom_Ellipse.hxx>
#include<Geom_Hyperbola.hxx>
#include<Geom_Parabola.hxx>
#include<Geom_BezierCurve.hxx>
#include<Geom_BSplineCurve.hxx>
#include<Geom_OffsetCurve.hxx>

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

			IXCurve^ XCurve::GeomToXCurve(Handle(Geom_Curve) curve)
			{
				Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
				if (!line.IsNull()) return gcnew XLine(line);
				Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
				if (!circle.IsNull()) return gcnew XCircle(circle);
				Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(curve);
				if (!ellipse.IsNull()) return gcnew XEllipse(ellipse);
				Handle(Geom_BSplineCurve) bSplineCurve = Handle(Geom_BSplineCurve)::DownCast(curve);
				if (!bSplineCurve.IsNull()) return gcnew XBSplineCurve(bSplineCurve);
				throw gcnew System::NotImplementedException("Curve Type not yet implemented as an interface");
				//TODO
				/*case GeomAbs_Hyperbola:
					break;
				case GeomAbs_Parabola:
					break;
				case GeomAbs_BezierCurve:
					break;
				case GeomAbs_OffsetCurve:
					break;
				case GeomAbs_OtherCurve:
					break;*/
			}


			IXPoint^ XCurve::GetPoint(double u)
			{
				gp_Pnt pnt;
				OccHandle()->D0(u, pnt);
				return gcnew XPoint(pnt);
			}
			IXPoint^ XCurve::GetSecondDerivative(double uParam, IXDirection^% direction, IXDirection^% normal)
			{
				gp_Pnt pnt;
				try
				{
					
					gp_Vec dir3d, norm3d;
					OccHandle()->D2(uParam, pnt, dir3d, norm3d);
					
					auto dir = gcnew XDirection(dir3d);
					direction = dir;
					auto norm = gcnew XDirection(norm3d.X(),norm3d.Y(),norm3d.Z());
					normal = norm;
					//normal can be null			
				}
				catch (const Standard_Failure&) //catch if we break the OCC rule that a direction cannot have all coords as 0
				{
					auto norm = gcnew XDirection();
					normal = norm;
				}
				return gcnew XPoint(pnt);
			}
			IXPoint^ XCurve::GetFirstDerivative(double u, IXDirection^% direction)
			{
				gp_Pnt pnt;
				gp_Vec vec;
				OccHandle()->D1(u, pnt, vec);
				auto dir = gcnew XDirection(vec);
				direction = dir;
				return gcnew XPoint(pnt);
			}
		}
	}
}