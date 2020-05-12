#include "XbimTrimmedCurve.h"
#include "XbimPoint.h"
#include "XbimVector.h"
#include "XbimLine.h"
#include <Geom_Line.hxx>
#include "XbimCircle.h"
#include <Geom_Circle.hxx>
#include "XbimEllipse.h"
#include <Geom_Ellipse.hxx>

#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXCurve^ XbimTrimmedCurve::BasisCurve::get()
			{				
				Handle(Geom_Curve) curve = OccHandle()->BasisCurve();
				
				Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
				if (!line.IsNull())
					return gcnew XbimLine(line);
				Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);
				if (!circle.IsNull())
					return gcnew XbimCircle(circle);
				Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(curve);
				if (!ellipse.IsNull())
					return gcnew XbimEllipse(ellipse);
				throw gcnew XbimGeometryFactoryException("Unsupported Trimmed Curve Basis");
			}

			IXPoint^ XbimTrimmedCurve::StartPoint::get()
			{							
				return gcnew XbimPoint(OccHandle()->StartPoint());
			}
			IXPoint^ XbimTrimmedCurve::EndPoint::get()
			{
				return gcnew XbimPoint(OccHandle()->EndPoint());
			}
			IXPoint^ XbimTrimmedCurve::GetPoint(double uParam)
			{
				gp_Pnt pnt; 
				OccHandle()->D0(uParam, pnt);
				return gcnew XbimPoint(pnt);
			}
			IXPoint^ XbimTrimmedCurve::GetFirstDerivative(double uParam, IXVector^% normal)
			{
				gp_Pnt pnt;
				gp_Vec vec;
				OccHandle()->D1(uParam, pnt, vec);
				normal = gcnew XbimVector(vec);
				return gcnew XbimPoint(pnt.X(), pnt.Y(), pnt.Z());
			}
		}
	}
}
