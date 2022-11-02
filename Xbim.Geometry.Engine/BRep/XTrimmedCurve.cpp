#include "XTrimmedCurve.h"
#include "XPoint.h"
#include "XVector.h"
#include "XLine.h"
#include <Geom_Line.hxx>
#include "XCircle.h"
#include <Geom_Circle.hxx>
#include "XEllipse.h"
#include <Geom_Ellipse.hxx>

#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXCurve^ XTrimmedCurve::BasisCurve::get()
			{				
				Handle(Geom_Curve) curve = OccTrimmedCurve()->BasisCurve();
				
				Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
				if (!line.IsNull())
					return gcnew XLine(line);
				Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
				if (!circle.IsNull())
					return gcnew XCircle(circle);
				Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(curve);
				if (!ellipse.IsNull())
					return gcnew XEllipse(ellipse);
				throw gcnew XbimGeometryFactoryException("Unsupported Trimmed Curve Basis");
			}

			IXPoint^ XTrimmedCurve::StartPoint::get()
			{							
				return gcnew XPoint(OccTrimmedCurve()->StartPoint());
			}
			IXPoint^ XTrimmedCurve::EndPoint::get()
			{
				return gcnew XPoint(OccTrimmedCurve()->EndPoint());
			}
			
		}
	}
}
