#include "XEdge.h"
#include "XVertex.h"
#include "XLine.h"
#include "XCircle.h"
#include "XEllipse.h"
#include "XTrimmedCurve.h"
#include "XBSplineCurve.h"
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS_Vertex.hxx>

#include "../Exceptions/XbimGeometryDefinitionException.h"
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			double XEdge::Length::get()
			{
				GProp_GProps gProps;
				BRepGProp::LinearProperties(OccEdge(), gProps);
				return gProps.Mass();
			};

			double XEdge::Tolerance::get()
			{
				return BRep_Tool::Tolerance(OccEdge());
			};

			IXVertex^ XEdge::EdgeStart::get()
			{		
				TopoDS_Vertex sv = TopExp::FirstVertex(OccEdge(), Standard_True);
				//if(sv.IsNull()) //it is possible for this to be null and it needs to be handled or not ever allowed to happen
				return gcnew XVertex(sv);
			}
			IXVertex^ XEdge::EdgeEnd::get()
			{
				TopoDS_Vertex ev = TopExp::LastVertex(OccEdge(), Standard_True);
				//if(sv.IsNull()) //it is possible for this to be null and it needs to be handled or not ever allowed to happen
				return gcnew XVertex(ev);
			}

			IXCurve^ XEdge::EdgeGeometry::get()
			{
				Standard_Real u1, u2;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(OccEdge(), u1, u2);
				//the curve may be null e.g. where it is a vertex loop (top point of a cone)
				if (curve.IsNull()) return nullptr;
				GeomAdaptor_Curve ga(curve,u1, u2);
				GeomAbs_CurveType curveType = ga.GetType();
				String^ curveTypeName="";
				switch (curveType)
				{
				case GeomAbs_Line:
					return gcnew XLine(Handle(Geom_Line)::DownCast(curve));
				case GeomAbs_Circle:
					return gcnew XCircle(Handle(Geom_Circle)::DownCast(curve));
				case GeomAbs_Ellipse:
					return gcnew XEllipse(Handle(Geom_Ellipse)::DownCast(curve));
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
					return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(curve));
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