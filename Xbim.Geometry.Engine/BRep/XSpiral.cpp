//#include "XCurve.h"
//#include "XSpiral.h"
//#include "XSpiral.h"
//#include "XSpiral.h"
//#include "XBSplineCurve.h"
//#include<Geom_Line.hxx>
////#include "./OccExtensions/Curves/Segments/Geom2d_Spiral.h"
//
//#include<Geom_Circle.hxx>
//#include<Geom_Ellipse.hxx>
//#include<Geom_Hyperbola.hxx>
//#include<Geom_Parabola.hxx>
//#include<Geom_BezierCurve.hxx>
//#include<Geom_BSplineCurve.hxx>
//#include<Geom_OffsetCurve.hxx>
//
//#include <gp_Pnt.hxx>
//#include "XPoint.h"
//#include "XVector.h"
//#include "XDirection.h"
//namespace Xbim
//{
//	namespace Geometry
//	{
//		namespace BRep
//		{
//			const Handle(Geom2d_Curve) XSpiral::GeomCurve(IXSpiral^ xCurve)
//			{
//				return ((XSpiral^)xCurve)->Ref();
//			}
//
//			IXPoint^ XSpiral::GetPoint(double u)
//			{
//				gp_Pnt2d pnt;
//				OccHandle()->D0(u, pnt);
//				return gcnew XPoint(pnt);
//			}
//
//			System::Nullable<double> XSpiral::GetCurvature(double uParam)
//			{
//				if (uParam < OccHandle()->FirstParameter()) return System::Nullable<double>();
//				if (uParam > OccHandle()->LastParameter()) return System::Nullable<double>();
//				return 0;// Handle(Geom2d_Spiral)::DownCast(OccHandle())->GetCurvatureAt(uParam);
//			}
//
//
//			IXPoint^ XSpiral::GetSecondDerivative(double uParam, IXDirection^% direction, IXDirection^% normal)
//			{
//				gp_Pnt2d pnt;
//				try
//				{
//
//					gp_Vec2d dir2d, norm2d;
//					OccHandle()->D2(uParam, pnt, dir2d, norm2d);
//
//					auto dir = gcnew XDirection(dir2d);
//					direction = dir;
//					auto norm = gcnew XDirection(norm2d.X(), norm2d.Y(), 0);
//					normal = norm;
//				}
//				catch (const Standard_Failure&)
//				{
//					auto norm = gcnew XDirection();
//					normal = norm;
//				}
//				return gcnew XPoint(pnt);
//			}
//			IXPoint^ XSpiral::GetFirstDerivative(double u, IXDirection^% direction)
//			{
//				gp_Pnt2d pnt;
//				gp_Vec2d vec;
//				OccHandle()->D1(u, pnt, vec);
//				auto dir = gcnew XDirection(vec);
//				direction = dir;
//				return gcnew XPoint(pnt);
//			}
//		}
//	}
//}