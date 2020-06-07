#include "XbimEdge.h"
#include "XbimVertex.h"
#include "XbimLine.h"
#include "XbimCircle.h"
#include "XbimEllipse.h"
#include "XbimTrimmedCurve.h"
#include "XbimBSplineCurve.h"
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS_Vertex.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include "../Exceptions/XbimGeometryDefinitionException.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			double XbimEdge::Tolerance::get()
			{
				return BRep_Tool::Tolerance(OccHandle());
			};

			IXVertex^ XbimEdge::EdgeStart::get()
			{		
				TopoDS_Vertex sv = TopExp::FirstVertex(OccHandle(), Standard_True);
				//if(sv.IsNull()) //it is possible for this to be null and it needs to be handled or not ever allowed to happen
				return gcnew XbimVertex(sv);
			}
			IXVertex^ XbimEdge::EdgeEnd::get()
			{
				TopoDS_Vertex ev = TopExp::LastVertex(OccHandle(), Standard_True);
				//if(sv.IsNull()) //it is possible for this to be null and it needs to be handled or not ever allowed to happen
				return gcnew XbimVertex(ev);
			}

			IXCurve^ XbimEdge::EdgeGeometry::get()
			{
				Standard_Real u1, u2;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(OccHandle(), u1, u2);
				//the curve may be null e.g. where it is a vertex loop (top point of a cone)
				if (curve.IsNull()) return nullptr;
				GeomAdaptor_HCurve ga(curve,u1, u2);
				GeomAbs_CurveType curveType = ga.GetType();
				String^ curveTypeName="";
				switch (curveType)
				{
				case GeomAbs_Line:
					return gcnew XbimLine(Handle(Geom_Line)::DownCast(curve));
				case GeomAbs_Circle:
					return gcnew XbimCircle(Handle(Geom_Circle)::DownCast(curve));
				case GeomAbs_Ellipse:
					return gcnew XbimEllipse(Handle(Geom_Ellipse)::DownCast(curve));
				case GeomAbs_Hyperbola:
					curveTypeName = gcnew String("GeomAbs_Hyperbola");
					break;
				case GeomAbs_Parabola:
					curveTypeName = gcnew String("GeomAbs_Parabola");
					break;
				case GeomAbs_BezierCurve:
					curveTypeName = gcnew String("GeomAbs_BezierCurve");
					break;
				case GeomAbs_BSplineCurve:
					return gcnew XbimBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(curve));
					break;
				case GeomAbs_OffsetCurve:
					curveTypeName = gcnew String("GeomAbs_OffsetCurve");
					break;
				case GeomAbs_OtherCurve:
					curveTypeName = gcnew String("GeomAbs_OtherCurve");
					break;
				default:
					curveTypeName = gcnew String("Unknown_CurveType");
					break;
				}			
				throw gcnew XbimGeometryDefinitionException(String::Format("Curve type not implemented: {0}", curveTypeName));
			}
		}
	}
}