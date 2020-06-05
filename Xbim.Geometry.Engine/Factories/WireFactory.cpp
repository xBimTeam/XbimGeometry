#include "WireFactory.h"
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS.hxx>
#include <Geom_Plane.hxx>
#include <TColgp_SequenceOfPnt2d.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <map>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <Geom_Ellipse.hxx>
#include "../BRep/OccExtensions/KeyedPnt.h"
#include <TColGeom_SequenceOfCurve.hxx>
using namespace System;
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
				case XCurveType::IfcPolyline: //build with no trim
					return BuildPolyline(static_cast<IIfcPolyline^>(ifcCurve), -1, -1);
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
				TopoDS_Wire wire = Ptr()->BuildPolyline2d(pointSeq, ModelService->Precision);
				return wire;
			}

			TopoDS_Wire WireFactory::BuildPolyline(IIfcPolyline^ ifcPolyline, double startParam, double endParam)
			{
				//validate
				if (!Enumerable::Any(ifcPolyline->Points))
					throw gcnew XbimGeometryFactoryException("IfcPolyline has zero points");
				NCollection_Vector<KeyedPnt> pointSeq;
				for each (IIfcCartesianPoint ^ cp  in ifcPolyline->Points) //these should be 2d points
				{
					pointSeq.Append(KeyedPnt(gp_XYZ(cp->X, cp->Y, cp->Z), cp->EntityLabel));
				}
				TopoDS_Wire wire = Ptr()->BuildPolyline(pointSeq, startParam, endParam, ModelService->Precision);
				return wire;
			}


			TopoDS_Wire WireFactory::BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam)
			{
				double sameParams = Math::Abs(endParam - startParam) < ModelService->Precision;
				if (sameParams || ((startParam == -1 || endParam == -1) && !_curveFactory->IsBoundedCurve(curve)))
					throw gcnew XbimGeometryFactoryException("DirectrixBounded: If the values for StartParam or EndParam are omited, then the Directrix has to be a bounded or closed curve.");
				if (3 != (int)curve->Dim)
					throw gcnew XbimGeometryFactoryException("DirectrixDim: The Directrix shall be a curve in three dimensional space.");
				XCurveType curveType;
				if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
					throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + curve->ExpressType->ExpressName);

				switch (curveType)
				{
					/*case Xbim::Geometry::Abstractions::XCurveType::BoundaryCurve:
						return Build3d(static_cast<IIfcBoundedCurve^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::BSplineCurveWithKnots:
						return Build3d(static_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case Xbim::Geometry::Abstractions::XCurveType::IfcCircle:
					return BuildDirectrix(static_cast<IIfcCircle^>(curve), startParam, endParam);
				case Xbim::Geometry::Abstractions::XCurveType::IfcEllipse:
					return BuildDirectrix(static_cast<IIfcEllipse^>(curve), startParam, endParam);
				case Xbim::Geometry::Abstractions::XCurveType::IfcLine:
					return BuildDirectrix(static_cast<IIfcLine^>(curve), startParam, endParam);
				case Xbim::Geometry::Abstractions::XCurveType::IfcPolyline:
					return BuildDirectrix(static_cast<IIfcPolyline^>(curve), startParam, endParam);
					/*case Xbim::Geometry::Abstractions::XCurveType::CompositeCurve:
						return Build3d(static_cast<IIfcCompositeCurve^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::CompositeCurveOnSurface:
						return Build3d(static_cast<IIfcCompositeCurveOnSurface^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::IndexedPolyCurve:
						return Build3d(static_cast<IIfcIndexedPolyCurve^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::OffsetCurve3D:
						return Build2d(static_cast<IIfcOffsetCurve3D^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::Pcurve:
						return Build3d(static_cast<IIfcPcurve^>(curve));

					case Xbim::Geometry::Abstractions::XCurveType::RationalBSplineCurveWithKnots:
						return Build3d(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::SurfaceCurve:
						return Build3d(static_cast<IIfcSurfaceCurve^>(curve));*/
				case Xbim::Geometry::Abstractions::XCurveType::IfcTrimmedCurve:
					return BuildDirectrix(static_cast<IIfcTrimmedCurve^>(curve), startParam, endParam);

				default:
					throw gcnew XbimGeometryFactoryException("Not implemented. Curve type: " + curveType.ToString());
				}

			}
			//this will never be called with an invalid start and end parameter as an ifcline is not a bounded curve and cannot be built as a directrix
			TopoDS_Wire WireFactory::BuildDirectrix(IIfcLine^ curve, double startParam, double endParam)
			{
				Handle(Geom_LineWithMagnitude) line = _curveFactory->BuildGeom3d(curve);
				if (line.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix is invalid");
				Handle(Geom_TrimmedCurve) trimmedLine = _curveFactory->Ptr()->BuildTrimmedCurve3d(line, startParam, endParam, (endParam > startParam));
				if (trimmedLine.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix could not be trimmed");
				return MakeWire(trimmedLine);
			}

			//this will never be called with an invalid start and end parameter as an ifcCircle is not a bounded curve and cannot be built as a directrix
			TopoDS_Wire WireFactory::BuildDirectrix(IIfcCircle^ curve, double startParam, double endParam)
			{
				Handle(Geom_Circle) circle = _curveFactory->BuildGeom3d(curve);
				if (circle.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix is invalid");
				if (Math::Abs(startParam - endParam) < ModelService->Precision) return MakeWire(circle);
				bool sameSense = (startParam < endParam);
				Handle(Geom_TrimmedCurve) trimmedLine = _curveFactory->Ptr()->BuildTrimmedCurve3d(circle, sameSense ? startParam : endParam, sameSense ? endParam : startParam, sameSense);
				if (trimmedLine.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix could not be trimmed");
				return MakeWire(trimmedLine);
			}

			//this will never be called with an invalid start and end parameter as an ifcElipse is not a bounded curve and cannot be built as a directrix
			TopoDS_Wire WireFactory::BuildDirectrix(IIfcEllipse^ curve, double startParam, double endParam)
			{
				Handle(Geom_Ellipse) elipse = _curveFactory->BuildGeom3d(curve);
				if (elipse.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix is invalid");
				if (Math::Abs(startParam - endParam) < ModelService->Precision) return MakeWire(elipse);
				bool sameSense = (startParam < endParam);
				Handle(Geom_TrimmedCurve) trimmedLine = _curveFactory->Ptr()->BuildTrimmedCurve3d(elipse, startParam, endParam, sameSense);
				if (trimmedLine.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix could not be trimmed");
				return MakeWire(trimmedLine);
			}

			TopoDS_Wire WireFactory::BuildDirectrix(IIfcTrimmedCurve^ curve, double startParam, double endParam)
			{
				Handle(Geom_TrimmedCurve) trimmed = _curveFactory->BuildGeom3d(curve);
				if (trimmed.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix is invalid");
				Handle(Geom_TrimmedCurve) trimmedLine = _curveFactory->Ptr()->BuildTrimmedCurve3d(trimmed, startParam, endParam, (endParam > startParam));
				if (trimmedLine.IsNull())
					throw gcnew XbimGeometryFactoryException("Directrix could not be trimmed");
				return MakeWire(trimmedLine);
			}

			TopoDS_Wire WireFactory::BuildDirectrix(IIfcPolyline^ curve, double startParam, double endParam)
			{
				TopoDS_Wire wire = BuildPolyline(curve, startParam, endParam);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Directrix is invalid");
				return wire;
			}
			TopoDS_Wire WireFactory::BuildDirectrix(IIfcCompositeCurve^ curve, double startParam, double endParam)
			{
				if (curve->NSegments < 1)
					throw gcnew XbimGeometryFactoryException("Directrix is invalid, no segments are defined");
				//build all the segments
				XCurveType curveType;
				TColGeom_SequenceOfCurve segments;

				for each (IIfcCompositeCurveSegment ^ segment in curve->Segments)
				{
					if (!_curveFactory->IsBoundedCurve(segment->ParentCurve))
						throw gcnew XbimGeometryFactoryException("Directrix is invalid, only curve segments that are bounded curves are permitted");
					Handle(Geom_Curve) hSegment = _curveFactory->BuildGeom3d(segment->ParentCurve, curveType);
					if (hSegment.IsNull())
						throw gcnew XbimGeometryFactoryException(String::Format("Directrix is invalid, curve segment #{0} is null", segment->EntityLabel));
					segments.Append(hSegment);
				}
				TopoDS_Wire wire = Ptr()->BuildCompositeCurve(segments);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Directrix is invalid, could not build wire");
				return wire;
			}
			TopoDS_Wire WireFactory::MakeWire(Handle(Geom_Curve) curve)
			{
				BRepBuilderAPI_MakeEdge edgeMaker(curve);
				BRep_Builder builder;
				TopoDS_Wire wire;
				builder.MakeWire(wire);
				builder.Add(wire, edgeMaker.Edge());
				return wire;
			}
		}
	}
}