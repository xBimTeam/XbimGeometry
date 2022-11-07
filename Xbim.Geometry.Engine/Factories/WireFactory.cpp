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
#include <TopExp_Explorer.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TopExp.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <TColGeom2d_SequenceOfCurve.hxx>
using namespace System;
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXWire^ WireFactory::BuildWire(array<IXPoint^>^ points)
			{
				//validate
				if (points->Length == 0)
					throw gcnew XbimGeometryFactoryException("Points has zero length");
				NCollection_Vector<KeyedPnt> pointSeq;
				int id = 0;
				IXPoint^ first = nullptr;
				IXPoint^ last = nullptr;
				for each (IXPoint ^ cp in points)
				{
					if (first == nullptr) first = cp;
					pointSeq.Append(KeyedPnt(gp_XYZ(cp->X, cp->Y, cp->Z), id++));
					last = cp;
				}
				TopoDS_Wire wire = Ptr()->BuildPolyline(pointSeq, -1, -1, ModelService->Precision);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Resulting wire is empty");
				if (GPFactory->IsEqual(first, last, ModelService->Precision))
					wire.Closed(true);
				return gcnew XWire(wire);
			}
			IXWire^ WireFactory::Build(IIfcCurve^ ifcCurve)
			{
				
				TopoDS_Wire wire = BuildWire(ifcCurve);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Resulting wire is empty");
				return gcnew XWire(wire);
			}

			IXWire^ WireFactory::Build(IIfcProfileDef^ ifcProfileDef)
			{
				TopoDS_Wire wire = BuildProfile(ifcProfileDef);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Resulting profile wire is empty");
				return gcnew XWire(wire);
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcCurve^ ifcCurve)
			{

				if ((int)ifcCurve->Dim == 2)
					return Build2d(ifcCurve);
				else
					return Build3d(ifcCurve);
			}

			double WireFactory::GetDeterminant(double x1, double y1, double x2, double y2)
			{
				return x1 * y2 - x2 * y1;
			}

			double WireFactory::Area(const TColgp_SequenceOfPnt2d& points2d)
			{
				if (points2d.Length() < 3)
				{
					return 0;
				}
				int nbPoints = (int)points2d.Length();
				double area = GetDeterminant(points2d.Value(nbPoints).X(), points2d.Value(nbPoints).Y(), points2d.Value(1).X(), points2d.Value(1).Y());

				for (int i = 2; i <= nbPoints; i++)
				{
					area += GetDeterminant(points2d.Value(i - 1).X(), points2d.Value(i - 1).Y(), points2d.Value(i).X(), points2d.Value(i).Y());
				}
				return (area / 2);
			}

			TopoDS_Wire WireFactory::Build2d(IIfcCurve^ ifcCurve)
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
					return Build2dCircle(static_cast<IIfcCircle^>(ifcCurve));
				case XCurveType::IfcCompositeCurve:
					return Build2dCompositeCurve(static_cast<IIfcCompositeCurve^>(ifcCurve));
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
					return Build2dPolyline(static_cast<IIfcPolyline^>(ifcCurve));
				case XCurveType::IfcRationalBSplineCurveWithKnots:
					break;
				case XCurveType::IfcSurfaceCurve:
					break;
				case XCurveType::IfcTrimmedCurve:
					return Build2dTrimmedCurve(static_cast<IIfcTrimmedCurve^>(ifcCurve));
				case XCurveType::IfcLine:
				default:
					throw gcnew XbimGeometryFactoryException("A wire cannot be built from a " + curveType.ToString());
					break;
				}
				throw gcnew XbimGeometryFactoryException("Not implemented. Curve type: " + curveType.ToString());
			}

			TopoDS_Wire WireFactory::Build3d(IIfcCurve^ ifcCurve)
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

			TopoDS_Wire WireFactory::Build2dCircle(IIfcCircle^ ifcCircle)
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
				
				return wire;
			}

			TopoDS_Wire WireFactory::Build2dTrimmedCurve(IIfcTrimmedCurve^ ifcTrimmedCurve)
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
				
				return wire;
			}

			TopoDS_Wire WireFactory::Build2dCompositeCurve(IIfcCompositeCurve^ ifcCompositeCurve)
			{
				TColGeom2d_SequenceOfCurve segments;
				Build2dSegments(ifcCompositeCurve, segments);
				return Ptr()->Build2dDirectrix(segments,-1,-1,_modelService->MinimumGap);
				
			}

			TopoDS_Wire WireFactory::Build2dPolyline(IIfcPolyline^ ifcPolyline)
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
				TopoDS_Wire wire = Ptr()->Build2dPolyline(pointSeq, ModelService->Precision);
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
				if (isTrimmedCurve) //special handle for IFC rules on trimmed segments, composite curve segment sense overrides the sense of the trim
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
			void WireFactory::Build2dSegments(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom2d_SequenceOfCurve& resultSegments, bool sameSense)
			{
				XCurveType curveType;
				TColGeom2d_SequenceOfCurve segments;
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
						segments.Append(Build2dCompositeCurveSegment(static_cast<IIfcCircle^>(segment->ParentCurve), segment->SameSense));
						break;
					case XCurveType::IfcEllipse:
						segments.Append(Build2dCompositeCurveSegment(static_cast<IIfcEllipse^>(segment->ParentCurve), segment->SameSense));
						break;
					case XCurveType::IfcLine:
						throw gcnew XbimGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
					case XCurveType::IfcPolyline:
					{
						IIfcPolyline^ pline = static_cast<IIfcPolyline^>(segment->ParentCurve);
						TColgp_Array1OfPnt2d points(1, pline->Points->Count);
						GPFactory->GetPolylinePoints2d(pline, points);
						Ptr()->GetPolylineSegments2d(points, segments, ModelService->Precision);
						break;
					}
					case XCurveType::IfcCompositeCurve:
						Build2dSegments(static_cast<IIfcCompositeCurve^>(segment->ParentCurve), segments, segment->SameSense);
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
						segments.Append(Build2dCompositeCurveSegment(static_cast<IIfcTrimmedCurve^>(segment->ParentCurve), segment->SameSense, true));
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
						Handle(Geom2d_Curve) seg = *it;
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
				if (endTrim.HasValue) endParam = endTrim.Value;
				double sameParams = Math::Abs(endParam - startParam) < ModelService->Precision;
				if (sameParams)
					throw  gcnew XbimGeometryFactoryException("Start and End Trim Parameters are the same. Null directrix is invalid");
				if (3 != (int)curve->Dim)
					LoggingService->LogInformation("DirectrixDim: The Directrix shall be a curve in three dimensional space. Automatic upgrade applied to 3D");
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
			template <typename IfcType>
			Handle(Geom2d_Curve) WireFactory::Build2dCompositeCurveSegment(IfcType ifcCurve, bool sameSense, bool isTrimmedCurve)
			{
				Handle(Geom2d_Curve) curve = _curveFactory->BuildGeom2d(ifcCurve);
				if (isTrimmedCurve) //special handle for IFC rules on trimmed segments, composite curve segment sense overrides the sense of the trim
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
				//make sure params are in radians
				startParam *= ModelService->RadianFactor;
				endParam *= ModelService->RadianFactor;
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
				//make sure params are in radians
				startParam *= ModelService->RadianFactor;
				endParam *= ModelService->RadianFactor;
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

			TopoDS_Wire WireFactory::BuildProfile(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileDefType;
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileDefType))
					throw gcnew XbimGeometryFactoryException("Unsupported curve type: " + profileDef->ExpressType->ExpressName);
				switch (profileDefType)
				{
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					break;
				case XProfileDefType::IfcArbitraryProfileDefWithVoids:
					break;
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					break;
				case XProfileDefType::IfcCenterLineProfileDef:
					break;
				case XProfileDefType::IfcCompositeProfileDef:
					break;
				case XProfileDefType::IfcDerivedProfileDef:
					break;
				case XProfileDefType::IfcMirroredProfileDef:
					break;
				case XProfileDefType::IfcAsymmetricIShapeProfileDef:
					break;
				case XProfileDefType::IfcCShapeProfileDef:
					break;
				case XProfileDefType::IfcCircleProfileDef:
					return BuildProfileDef(static_cast<IIfcCircleProfileDef^>(profileDef));
				case XProfileDefType::IfcCircleHollowProfileDef:
					break;
				case XProfileDefType::IfcEllipseProfileDef:
					break;
				case XProfileDefType::IfcIShapeProfileDef:
					break;
				case XProfileDefType::IfcLShapeProfileDef:
					break;
				case XProfileDefType::IfcRectangleProfileDef:
					return BuildProfileDef(static_cast<IIfcRectangleProfileDef^>(profileDef));
				case XProfileDefType::IfcRectangleHollowProfileDef:

					break;
				case XProfileDefType::IfcRoundedRectangleProfileDef:
					break;
				case XProfileDefType::IfcTShapeProfileDef:
					break;
				case XProfileDefType::IfcTrapeziumProfileDef:
					break;
				case XProfileDefType::IfcUShapeProfileDef:
					break;
				case XProfileDefType::IfcZShapeProfileDef:
					break;
				default:
					break;
				}
				throw gcnew XbimGeometryFactoryException("Not implemented. Profile Definition Type type: " + profileDefType.ToString());
			}

			TopoDS_Wire WireFactory::BuildProfileDef(IIfcRectangleProfileDef^ rectProfile)
			{
				if (rectProfile->XDim <= 0 || rectProfile->YDim <= 0)
					throw gcnew XbimGeometryFactoryException("Rectangle profile has an invalid dimension <= 0");
				TopoDS_Wire wire = Ptr()->BuildRectangleProfileDef(rectProfile->XDim, rectProfile->YDim);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Failure to build Rectangle Profile Def");
				//apply the position transformation
				if (rectProfile->Position != nullptr)
					wire.Move(GPFactory->ToLocation(rectProfile->Position));
				return wire;
			}

			TopoDS_Wire WireFactory::BuildProfileDef(IIfcCircleProfileDef^ circleProfile)
			{
				if (circleProfile->Radius <= 0)
					throw gcnew XbimGeometryFactoryException("Circle profile definition has an invalid dimension radius <= 0");
				gp_Ax22d position = GPFactory->BuildAxis2Placement2d(circleProfile->Position);

				TopoDS_Wire wire = Ptr()->BuildCircleProfileDef(circleProfile->Radius, position);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw gcnew XbimGeometryFactoryException("Failure to build Rectangle Profile Def");
				//apply the position transformation
				if (circleProfile->Position != nullptr)
					wire.Move(GPFactory->ToLocation(circleProfile->Position));
				return wire;
			}

			bool WireFactory::GetNormal(const TopoDS_Wire& wire, gp_Vec& normal)
			{

				return Ptr()->GetNormal(wire, normal);
			}

			double WireFactory::Area(const TopoDS_Wire& wire)
			{
				return Ptr()->Area(wire);
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