#include "XbimTrimmedCurve2d.h"
#include "Xbim2dPoint.h"
#include "Xbim2dVector.h"
#include <Geom2d_Line.hxx>
#include "XbimLine2d.h"
#include <Geom2d_Circle.hxx>
#include "XbimCircle2d.h"
#include <Geom2d_Ellipse.hxx>
#include "XbimEllipse2d.h"

#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXCurve^ XbimTrimmedCurve2d::BasisCurve::get()
			{
				Handle(Geom2d_Curve) curve = OccHandle()->BasisCurve();

				Handle(Geom2d_LineWithMagnitude) line = Handle(Geom2d_LineWithMagnitude)::DownCast(curve);
				if (!line.IsNull())
					return gcnew XbimLine2d(line);
				Handle(Geom2d_Circle) circle = Handle(Geom2d_Circle)::DownCast(curve);
				if (!circle.IsNull())
					return gcnew XbimCircle2d(circle);
				Handle(Geom2d_Ellipse) ellipse = Handle(Geom2d_Ellipse)::DownCast(curve);
				if (!ellipse.IsNull())
					return gcnew XbimEllipse2d(ellipse);
				throw gcnew XbimGeometryFactoryException("Unsupported Trimmed Curve Basis");
			}

			IXPoint^ XbimTrimmedCurve2d::StartPoint::get()
			{
				return gcnew Xbim2dPoint(OccHandle()->StartPoint());
			}
			IXPoint^ XbimTrimmedCurve2d::EndPoint::get()
			{
				return gcnew Xbim2dPoint(OccHandle()->EndPoint());
			}
			IXPoint^ XbimTrimmedCurve2d::GetPoint(double uParam)
			{
				gp_Pnt2d pnt;
				OccHandle()->D0(uParam, pnt);
				return gcnew Xbim2dPoint(pnt);
			}
			IXPoint^ XbimTrimmedCurve2d::GetFirstDerivative(double uParam, IXVector^% normal)
			{
				gp_Pnt2d pnt;
				gp_Vec2d vec;
				OccHandle()->D1(uParam, pnt, vec);
				normal = gcnew Xbim2dVector(vec);
				return gcnew Xbim2dPoint(pnt.X(), pnt.Y());
			}
		}
	}
}
