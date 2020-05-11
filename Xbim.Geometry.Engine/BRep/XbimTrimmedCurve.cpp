#include "XbimTrimmedCurve.h"
#include "XbimPoint.h"
#include "XbimVector.h"
#include "XbimLine.h"
#include <Geom_Line.hxx>
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
				
				Handle(Geom_LineWithMagnitude) line = Handle(Geom_LineWithMagnitude)::DownCast(curve);
				if (!line.IsNull())
					return gcnew XbimLine(line);
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
