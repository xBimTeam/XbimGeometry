#include "WireFactory.h"
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <TopoDS.hxx>
#include <Geom_Plane.hxx>
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			TopoDS_Wire WireFactory::Build(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& hSurface)
			{
				
				if ((int)ifcCurve->Dim == 2) 
					return Build2d(ifcCurve, hSurface);
				else 
					return Build3d(ifcCurve, hSurface);
			}
			TopoDS_Wire WireFactory::Build2d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& hSurface)
			{
				//validate
				if ((int)ifcCurve->Dim != 2)
					throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d curve from a 3d definition");
				XCurveType curveType;
				if (!Enum::TryParse<XCurveType>(ifcCurve->ExpressType->ExpressName, curveType))
					throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + ifcCurve->ExpressType->ExpressName);				
				switch (curveType)
				{
				case XCurveType::IfcBoundaryCurve:
					break;
				case XCurveType::IfcBSplineCurveWithKnots:
					break;
				case XCurveType::IfcCircle:
					return Build2dCircle(static_cast<IIfcCircle^>(ifcCurve), hSurface);
				case XCurveType::IfcCompositeCurve:
					break;
				case XCurveType::IfcCompositeCurveOnSurface:
					break;
				case XCurveType::IfcEllipse:
					break;
				case XCurveType::IfcIndexedPolyCurve:
					break;
				case XCurveType::IfcLine:
					break;
				case XCurveType::IfcOffsetCurve3D:
					break;
				case XCurveType::IfcOffsetCurve2D:
					break;
				case XCurveType::IfcPcurve:
					break;
				case XCurveType::IfcPolyline:
					break;
				case XCurveType::IfcRationalBSplineCurveWithKnots:
					break;
				case XCurveType::IfcSurfaceCurve:
					break;
				case XCurveType::IfcTrimmedCurve:
					break;
				default:
					break;
				}
				throw gcnew XbimGeometryFactoryException("Not implemented. Curve type: " + curveType.ToString());
			}
			TopoDS_Wire WireFactory::Build3d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface)
			{
				XCurveType curveType;
				if (!Enum::TryParse<XCurveType>(ifcCurve->ExpressType->ExpressName, curveType))
					throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + ifcCurve->ExpressType->ExpressName);
				switch (curveType)
				{
				case XCurveType::IfcBoundaryCurve:
					break;
				case XCurveType::IfcBSplineCurveWithKnots:
					break;
				case XCurveType::IfcCircle:
					break;
				case XCurveType::IfcCompositeCurve:
					break;
				case XCurveType::IfcCompositeCurveOnSurface:
					break;
				case XCurveType::IfcEllipse:
					break;
				case XCurveType::IfcIndexedPolyCurve:
					break;
				case XCurveType::IfcLine:
					break;
				case XCurveType::IfcOffsetCurve3D:
					break;
				case XCurveType::IfcOffsetCurve2D:
					break;
				case XCurveType::IfcPcurve:
					break;
				case XCurveType::IfcPolyline:
					break;
				case XCurveType::IfcRationalBSplineCurveWithKnots:
					break;
				case XCurveType::IfcSurfaceCurve:
					break;
				case XCurveType::IfcTrimmedCurve:
					break;
				default:
					break;
				}
				throw gcnew XbimGeometryFactoryException("Not implemented. Curve type: " + curveType.ToString());
			}
			TopoDS_Wire WireFactory::Build2dCircle(IIfcCircle^ ifcCircle, Handle(Geom_Surface)& surface)
			{
				Handle(Geom2d_Curve) hCurve = _curveFactory->BuildGeom2d(ifcCircle);
				BRepBuilderAPI_MakeEdge2d edgeMaker(hCurve);
				if (!edgeMaker.IsDone()) throw gcnew XbimGeometryFactoryException("Failed to build edge");
				TopoDS_Wire wire;
				BRep_Builder builder;
				builder.MakeWire(wire);
				builder.Add(wire, TopoDS::Edge(edgeMaker.Shape()));
				//must be a planar surface
				Handle(Geom2d_Circle) circ = Handle(Geom2d_Circle)::DownCast(hCurve);
				gp_Pnt2d centre = circ->Location();
				surface = new Geom_Plane(gp_Pnt(centre.X(), centre.Y(), 0.0),gp::DZ());
				return wire;
			}
		}
	}
}