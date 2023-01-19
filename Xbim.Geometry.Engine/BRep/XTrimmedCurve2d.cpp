#include "XTrimmedCurve2d.h"
#include "X2dPoint.h"
#include "X2dVector.h"
#include <Geom2d_Line.hxx>
#include "XLine2d.h"
#include <Geom2d_Circle.hxx>
#include "XCircle2d.h"
#include <Geom2d_Ellipse.hxx>
#include "XEllipse2d.h"


using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXCurve^ XTrimmedCurve2d::BasisCurve::get()
			{
				Handle(Geom2d_Curve) curve = OccTrimmedCurve2d()->BasisCurve();

				Handle(Geom2d_LineWithMagnitude) line = Handle(Geom2d_LineWithMagnitude)::DownCast(curve);
				if (!line.IsNull())
					return gcnew XLine2d(line);
				Handle(Geom2d_Circle) circle = Handle(Geom2d_Circle)::DownCast(curve);
				if (!circle.IsNull())
					return gcnew XCircle2d(circle);
				Handle(Geom2d_Ellipse) ellipse = Handle(Geom2d_Ellipse)::DownCast(curve);
				if (!ellipse.IsNull())
					return gcnew XEllipse2d(ellipse);
				throw gcnew XbimGeometryFactoryException("Unsupported Trimmed Curve Basis");
			}

			IXPoint^ XTrimmedCurve2d::StartPoint::get()
			{
				return gcnew X2dPoint(OccTrimmedCurve2d()->StartPoint());
			}
			IXPoint^ XTrimmedCurve2d::EndPoint::get()
			{
				return gcnew X2dPoint(OccTrimmedCurve2d()->EndPoint());
			}
			
		}
	}
}
