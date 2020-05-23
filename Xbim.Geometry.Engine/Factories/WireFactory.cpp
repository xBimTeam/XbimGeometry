#include "WireFactory.h"
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <TopoDS.hxx>
#include <Geom_Plane.hxx>
#include <TColgp_SequenceOfPnt2d.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <map>
#include <TopTools_DataMapOfIntegerShape.hxx>

using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXWire^ WireFactory::Build(IIfcCurve^ ifcCurve)
			{
				Handle(Geom_Surface) surface;
				TopoDS_Wire wire = BuildWire(ifcCurve, surface);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Resulting wire is empty");
				return gcnew XbimWire(wire);
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface)
			{

				if ((int)ifcCurve->Dim == 2)
					return Build2d(ifcCurve, surface);
				else
					return Build3d(ifcCurve, surface);
			}

			TopoDS_Wire WireFactory::Build2d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface)
			{
				//validate
				if ((int)ifcCurve->Dim != 2)
					throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d curve from a 3d definition");
				//a wire can only be built from a bounded or closed curve, we check below

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
					return Build2dCircle(static_cast<IIfcCircle^>(ifcCurve), surface);
				case XCurveType::IfcCompositeCurve:
					break;
				case XCurveType::IfcCompositeCurveOnSurface:
					break;
				case XCurveType::IfcEllipse:
					break;
				case XCurveType::IfcIndexedPolyCurve:
					break;
				case XCurveType::IfcOffsetCurve3D:
					break;
				case XCurveType::IfcOffsetCurve2D:
					break;
				case XCurveType::IfcPcurve:
					break;
				case XCurveType::IfcPolyline:
					return Build2dPolyline(static_cast<IIfcPolyline^>(ifcCurve), surface);
				case XCurveType::IfcRationalBSplineCurveWithKnots:
					break;
				case XCurveType::IfcSurfaceCurve:
					break;
				case XCurveType::IfcTrimmedCurve:
					return Build2dTrimmedCurve(static_cast<IIfcTrimmedCurve^>(ifcCurve), surface);
				case XCurveType::IfcLine:
				default:
					throw gcnew XbimGeometryFactoryException("A wire cannot be built from a " + curveType.ToString());
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
				surface = new Geom_Plane(gp_Pnt(centre.X(), centre.Y(), 0.0), gp::DZ());
				return wire;
			}

			TopoDS_Wire WireFactory::Build2dTrimmedCurve(IIfcTrimmedCurve^ ifcTrimmedCurve, Handle(Geom_Surface)& surface)
			{
				Handle(Geom2d_Curve) hCurve = _curveFactory->BuildGeom2d(ifcTrimmedCurve);
				BRepBuilderAPI_MakeEdge2d edgeMaker(hCurve);
				if (!edgeMaker.IsDone()) throw gcnew XbimGeometryFactoryException("Failed to build edge");
				TopoDS_Wire wire;
				BRep_Builder builder;
				builder.MakeWire(wire);
				builder.Add(wire, TopoDS::Edge(edgeMaker.Shape()));
				//must be a planar surface
				Handle(Geom2d_TrimmedCurve) trim = Handle(Geom2d_TrimmedCurve)::DownCast(hCurve);
				gp_Pnt2d start = trim->StartPoint();
				surface = new Geom_Plane(gp_Pnt(start.X(), start.Y(), 0.0), gp::DZ());
				return wire;
			}

			TopoDS_Wire WireFactory::Build2dPolyline(IIfcPolyline^ ifcPolyline, Handle(Geom_Surface)& surface)
			{
				//validate
				if (!Enumerable::Any(ifcPolyline->Points))
					throw gcnew XbimGeometryFactoryException("IfcPolyline has zero points");
				//we don't treat a polyline as a curve when we are building a wire from a polyline 
				//as generally it needs to be a compound of edges and vertices, only as a brep edge do we use the curve methodology
				//std::map<int, gp_Pnt2d> mapOfPoints; //the key is the ifc entity label
				//get a list of points but remove duplicates by using the tolerance
				NCollection_Vector<KeyedPnt2d> pointSeq;
				for each (IIfcCartesianPoint ^ cp  in ifcPolyline->Points) //these should be 2d points
				{
					pointSeq.Append(KeyedPnt2d(gp_XY(cp->X, cp->Y), cp->EntityLabel));
				}
				
				return Ptr()->BuildPolyline(pointSeq, ModelService->Precision);

				//if (closed && pointSeq.size() < 4)
				//	LoggerService->LogWarning("A closed polyline should not have less than four points"); //it will be built as open

				//if(pointSeq.size() < 2)
				//	throw gcnew XbimGeometryFactoryException("IfcPolyline is a single point");

				//BRepBuilderAPI_MakePolygon polyMaker;
				//for (size_t i = 1; i <= pointSeq.size(); i++)
				//{
				//	gp_Pnt2d pnt2d = pointSeq.size(i);
				//	polyMaker.Add(gp_Pnt(pnt2d.X(), pnt2d.Y(), .0));

				//}
				//if (!polyMaker.IsDone()) throw gcnew XbimGeometryFactoryException("Failed to build edge");
				//TopoDS_Wire wire;
				//BRep_Builder builder;
				//builder.MakeWire(wire);
				//////builder.Add(wire, TopoDS::Edge(edgeMaker.Shape()));
				//////must be a planar surface
				////
				////gp_Pnt2d start = pointSeq.First();
				////surface = new Geom_Plane(gp_Pnt(start.X(), start.Y(), 0.0), gp::DZ());
				//return wire;
			}
		}
	}
}