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
					return BuildDirectrix(static_cast<IIfcCompositeCurve^>(ifcCurve), Nullable<IfcParameterValue>(), Nullable<IfcParameterValue>());
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
					return BuildDirectrix(static_cast<IIfcPolyline^>(ifcCurve), Nullable<IfcParameterValue>(), Nullable<IfcParameterValue>());
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

			TopoDS_Wire WireFactory::Build3D(IIfcPolyline^ ifcPolyline, double startParam, double endParam)
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


			template <typename IfcType>
			Handle(Geom_Curve) WireFactory::BuildCompositeCurveSegment(IfcType ifcCurve, bool sameSense, bool isTrimmedCurve)
			{
				Handle(Geom_Curve) curve = _curveFactory->BuildGeom3d(ifcCurve);
				if (isTrimmedCurve) //special handle for IFC rules on trimmed segments
				{
					IIfcTrimmedCurve^ tc = dynamic_cast<IIfcTrimmedCurve^>(ifcCurve);
					if (!sameSense)
					{
						if (tc->SenseAgreement)
						{
							curve->Reverse();
						}
					}
					else
					{
						if (!tc->SenseAgreement)
						{
							curve->Reverse();
						}
					}
				}
				else
					if (!sameSense) curve->Reverse();
				return curve;
			}


			void WireFactory::BuildSegments(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom_SequenceOfCurve& resultSegments, bool sameSense)
			{
				XCurveType curveType;
				TColGeom_SequenceOfCurve segments;
				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{

					if (!Enum::TryParse<XCurveType>(segment->ParentCurve->ExpressType->ExpressName, curveType))
						throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + segment->ParentCurve->ExpressType->ExpressName);
					Handle(Geom_Curve) curve;
					switch (curveType)
					{
						/*case XCurveType::BoundaryCurve:
							return Build3d(static_cast<IIfcBoundedCurve^>(curve));
						case XCurveType::BSplineCurveWithKnots:
							return Build3d(static_cast<IIfcBSplineCurveWithKnots^>(curve));*/
					case XCurveType::IfcCircle:
						segments.Append(BuildCompositeCurveSegment(static_cast<IIfcCircle^>(segment->ParentCurve), segment->SameSense));
						break;
					case XCurveType::IfcEllipse:
						segments.Append(BuildCompositeCurveSegment(static_cast<IIfcEllipse^>(segment->ParentCurve), segment->SameSense));
						break;
					case XCurveType::IfcLine:
						throw gcnew XbimGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
					case XCurveType::IfcPolyline:
					{
						IIfcPolyline^ pline = static_cast<IIfcPolyline^>(segment->ParentCurve);
						TColgp_Array1OfPnt points(1, pline->Points->Count);
						GPFactory->GetPolylinePoints(pline, points);
						Ptr()->GetPolylineSegments(points, segments, ModelService->Precision);
						break;
					}
					case XCurveType::IfcCompositeCurve:
						BuildSegments(static_cast<IIfcCompositeCurve^>(segment->ParentCurve), segments, segment->SameSense);
						break;
						/*case XCurveType::CompositeCurveOnSurface:
							return Build3d(static_cast<IIfcCompositeCurveOnSurface^>(curve));
						case XCurveType::IndexedPolyCurve:
							return Build3d(static_cast<IIfcIndexedPolyCurve^>(curve));
						case XCurveType::OffsetCurve3D:
							return Build2d(static_cast<IIfcOffsetCurve3D^>(curve));
						case XCurveType::Pcurve:
							return Build3d(static_cast<IIfcPcurve^>(curve));

						case XCurveType::RationalBSplineCurveWithKnots:
							return Build3d(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
						case XCurveType::SurfaceCurve:
							return Build3d(static_cast<IIfcSurfaceCurve^>(curve));*/
					case XCurveType::IfcTrimmedCurve:
						segments.Append(BuildCompositeCurveSegment(static_cast<IIfcTrimmedCurve^>(segment->ParentCurve), segment->SameSense, true));
						break;
					default:
						throw gcnew XbimGeometryFactoryException("Not implemented. Curve type: " + curveType.ToString());
					}
				}
				//if this is a nested composite curve ensure sense is applied
				for (auto it = segments.cbegin(); it != segments.cend(); ++it)
				{
					if (sameSense)
						resultSegments.Append(*it);
					else
					{
						Handle(Geom_Curve) seg = *it;
						seg->Reverse();
						resultSegments.Append(seg);
					}
				}
			}

			TopoDS_Wire WireFactory::BuildDirectrix(IIfcCurve^ curve, Nullable<IfcParameterValue> startTrim, Nullable<IfcParameterValue> endTrim)
			{
				if ((!startTrim.HasValue || !endTrim.HasValue) && !_curveFactory->IsBoundedCurve(curve))
					throw gcnew XbimGeometryFactoryException("DirectrixBounded: If the values for StartParam or EndParam are omited, then the Directrix has to be a bounded or closed curve.");

				double startParam = 0;
				double endParam = double::PositiveInfinity;
				if (startTrim.HasValue) startParam = startTrim.Value;
				if (endTrim.HasValue) startParam = endTrim.Value;
				double sameParams = Math::Abs(endParam - startParam) < ModelService->Precision;
				if (sameParams)
					throw  gcnew XbimGeometryFactoryException("Start and End Trim Parameters are the same. Null directrix is invalid");
				if (3 != (int)curve->Dim)
					LoggerService->LogInformation("DirectrixDim: The Directrix shall be a curve in three dimensional space. Automatic upgrade applied to 3D");
				XCurveType curveType;
				if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
					throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + curve->ExpressType->ExpressName);

				switch (curveType)
				{
					/*case XCurveType::BoundaryCurve:
						return Build3d(static_cast<IIfcBoundedCurve^>(curve));
					case XCurveType::BSplineCurveWithKnots:
						return Build3d(static_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case XCurveType::IfcCircle:
					return BuildDirectrix(static_cast<IIfcCircle^>(curve), startParam, endParam);
				case XCurveType::IfcEllipse:
					return BuildDirectrix(static_cast<IIfcEllipse^>(curve), startParam, endParam);
				case XCurveType::IfcLine:
					return BuildDirectrix(static_cast<IIfcLine^>(curve), startParam, endParam);
				case XCurveType::IfcPolyline:
					return BuildDirectrix(static_cast<IIfcPolyline^>(curve), startParam, endParam);
				case XCurveType::IfcCompositeCurve:
					return BuildDirectrix(static_cast<IIfcCompositeCurve^>(curve), startParam, endParam);
					/*case XCurveType::CompositeCurveOnSurface:
						return Build3d(static_cast<IIfcCompositeCurveOnSurface^>(curve));
					case XCurveType::IndexedPolyCurve:
						return Build3d(static_cast<IIfcIndexedPolyCurve^>(curve));
					case XCurveType::OffsetCurve3D:
						return Build2d(static_cast<IIfcOffsetCurve3D^>(curve));
					case XCurveType::Pcurve:
						return Build3d(static_cast<IIfcPcurve^>(curve));

					case XCurveType::RationalBSplineCurveWithKnots:
						return Build3d(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::SurfaceCurve:
						return Build3d(static_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
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
				TopoDS_Wire wire = Build3D(curve, startParam, endParam);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Directrix is invalid");
				return wire;
			}

			TopoDS_Wire WireFactory::BuildDirectrix(IIfcCompositeCurve^ curve, double startParam, double endParam)
			{

				TColGeom_SequenceOfCurve segments;
				BuildSegments(curve, segments);
				TopoDS_Wire wire = Ptr()->BuildDirectrix(segments, startParam, endParam, ModelService->Precision);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Directrix is an empty curve");
				return wire;
			}

			//TopoDS_Wire CurveFactory::BuildGeom3d(IIfcCompositeCurve^ ifcCompositeCurve)
			//{
			//	XCurveType curveType;
			//	TColGeom_SequenceOfCurve segments;

			//	for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
			//	{
			//		if (!IsBoundedCurve(segment->ParentCurve))
			//			throw gcnew XbimGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
			//		Handle(Geom_Curve) hSegment = BuildGeom3d(segment->ParentCurve, curveType);
			//		Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(hSegment);
			//		if (boundedCurve.IsNull())
			//			gcnew XbimGeometryFactoryException(String::Format("Compound curve segments must be bounded curves #{0}", segment->EntityLabel));
			//		if (hSegment.IsNull())
			//			throw gcnew XbimGeometryFactoryException(String::Format("Composite curve is invalid, curve segment #{0} is null", segment->EntityLabel));
			//		segments.Append(boundedCurve);
			//	}
			//	Handle(Geom_BSplineCurve) bSpline = Ptr()->BuildCompositeCurve(segments, ModelService->MinimumGap); //use minimum gap for tolerance to avoid issues with cirves and line tolerance errors
			//	if (bSpline.IsNull())
			//		throw gcnew XbimGeometryFactoryException(String::Format("Composite curve #{0} could not be built", ifcCompositeCurve->EntityLabel));
			//	else
			//		return bSpline;
			//}
			//void WireFactory::GetCurves(IIfcPolyline^ ifcPolyline, TColGeom_SequenceOfCurve& curves)
			//{
			//	//validate
			//	int pointCount = ifcPolyline->Points->Count;
			//	if (pointCount < 2)
			//	{
			//		LoggerService->LogWarning("IfcPolyline has less than 2 points. It has been skipped");
			//		return;
			//	}
			//	if (pointCount == 2) //just build a line
			//	{
			//		gp_Pnt start = GPFactory->BuildPoint(ifcPolyline->Points[0]);
			//		gp_Pnt end = GPFactory->BuildPoint(ifcPolyline->Points[1]);
			//		if (start.IsEqual(end, ModelService->Precision))
			//		{
			//			LoggerService->LogWarning(String::Format("IfcPolyline has only 2 identical points( #{0} and #{1}. It has been skippedt", ifcPolyline->Points[0]->EntityLabel, ifcPolyline->Points[1]->EntityLabel));
			//			return;
			//		}
			//		Handle(Geom_TrimmedCurve) lineSeg = _curveFactory->Ptr()->BuildBoundedLine3d(start, end);
			//		if (lineSeg.IsNull())
			//			throw gcnew XbimGeometryFactoryException("Invalid IfcPolyline definition");
			//		curves.Append(lineSeg);
			//		return;
			//	}
			//	else
			//	{
			//		TColgp_Array1OfPnt points(1, pointCount);
			//		int i = 1;
			//		for each (IIfcCartesianPoint ^ ifcPoint in ifcPolyline->Points)
			//		{
			//			gp_Pnt pnt = GPFactory->BuildPoint(ifcPoint);
			//			points.SetValue(i, pnt);
			//			i++;
			//		}

			//		Ptr()->GetCurves(points, curves, ModelService->Precision);
			//		if (polyline.IsNull())
			//			throw gcnew XbimGeometryFactoryException("Failed to build IfcPolyline");
			//		return polyline;
			//	}
			//}
			//void WireFactory::GetCurves(IIfcCompositeCurve^ compCurve, TColGeom_SequenceOfCurve& curves)
			//{

			//}
			//void WireFactory::GetCurves(IIfcCompositeCurveOnSurface^ compCurve, TColGeom_SequenceOfCurve& curves)
			//{
			//	throw gcnew NotImplementedException();
			//}
			//void WireFactory::GetCurves(IIfcIndexedPolyCurve^ compCurve, TColGeom_SequenceOfCurve& curves)
			//{
			//	throw gcnew NotImplementedException();
			//}
			//void WireFactory::GetCurves(IIfcCurve^ curve, TColGeom_SequenceOfCurve& curves)
			//{
			//	XCurveType curveType;
			//	if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
			//		throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + curve->ExpressType->ExpressName);
			//	switch (curveType)
			//	{
			//	case XCurveType::IfcCompositeCurve:
			//		return GetCurves(static_cast<IIfcCompositeCurve^>(curve), curves);
			//	case XCurveType::IfcCompositeCurveOnSurface:
			//		return GetCurves(static_cast<IIfcCompositeCurveOnSurface^>(curve), curves);
			//	case XCurveType::IfcIndexedPolyCurve:
			//		return GetCurves(static_cast<IIfcIndexedPolyCurve^>(curve), curves);
			//	case XCurveType::IfcPolyline:
			//		return GetCurves(static_cast<IIfcPolyline^>(curve), curves);
			//	default:
			//		curves.Append(_curveFactory->BuildGeom3d(curve, curveType)); //just build the curve it is a single curve
			//		break;
			//	}
			//}


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