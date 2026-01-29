#include "WireFactory.h"
#include "CurveFactory.h"
#include "GeometryFactory.h"
#include "EdgeFactory.h"
#include "ProfileFactory.h"
#include "BIMAuthoringToolWorkArounds.h"
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

#include "../BRep/XWire.h"
#include <GeomLib_Tool.hxx>

using namespace System;
using namespace System::Linq;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4::MeasureResource;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{




			TopoDS_Wire WireFactory::BuildWire(const TopTools_SequenceOfShape& edgeList)
			{
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edgeList);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Edge list could not be built as a wire");
				return wire;
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcCurve^ ifcCurve, bool asSingleEdge)
			{
				XCurveType curveType;
				if (!Enum::TryParse<XCurveType>(ifcCurve->ExpressType->ExpressName, curveType))
					throw RaiseGeometryFactoryException("Unsupported curve type", ifcCurve);

				switch (curveType)
				{
				case XCurveType::IfcBSplineCurveWithKnots:
					return BuildWire(static_cast<IIfcBSplineCurveWithKnots^>(ifcCurve), true);
				case XCurveType::IfcCircle:
					return BuildWire(static_cast<IIfcCircle^>(ifcCurve));
				case XCurveType::IfcCompositeCurve:
					return BuildWire(static_cast<IIfcCompositeCurve^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcEllipse:
					return BuildWire(static_cast<IIfcEllipse^>(ifcCurve));
				case XCurveType::IfcIndexedPolyCurve:
					return BuildWire(static_cast<IIfcIndexedPolyCurve^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcLine:
					return BuildWire(static_cast<IIfcLine^>(ifcCurve));
				case XCurveType::IfcOffsetCurve2D:
					return BuildWire(static_cast<IIfcOffsetCurve2D^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcOffsetCurve3D:
					return BuildWire(static_cast<IIfcOffsetCurve3D^>(ifcCurve), asSingleEdge);
					/*case XCurveType::IfcPcurve:
						return BuildCurve2d(static_cast<IIfcPcurve^>(curve));*/
				case XCurveType::IfcPolyline:
					return BuildWire(static_cast<IIfcPolyline^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcRationalBSplineCurveWithKnots:
					return BuildWire(static_cast<IIfcRationalBSplineCurveWithKnots^>(ifcCurve));
					/*case XCurveType::IfcSurfaceCurve:
						return BuildCurve2d(static_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
					return BuildWire(static_cast<IIfcTrimmedCurve^>(ifcCurve), asSingleEdge);

				default:
					throw RaiseGeometryFactoryException("Unsupported curve type", ifcCurve);
				}

			}

			TopoDS_Wire WireFactory::BuildWire(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcBSplineCurveWithKnots); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Circle could not be built as a wire", ifcBSplineCurveWithKnots);
				return wire;
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcCircle^ ifcCircle)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcCircle); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Circle could not be built as a wire", ifcCircle);
				return wire;
			}



			TopoDS_Wire WireFactory::BuildWire(IIfcEllipse^ ifcEllipse)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcEllipse); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcEllipse could not be built as a wire", ifcEllipse);
				return wire;
			}



			TopoDS_Wire WireFactory::BuildWire(IIfcLine^ ifcLine)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcLine); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcLine could not be built as a wire", ifcLine);
				return wire;
			}


			TopoDS_Wire WireFactory::BuildWire(IIfcOffsetCurve2D^ ifcOffsetCurve2D, bool asSingleEdge)
			{
				TopoDS_Wire basisWire = BuildWire(ifcOffsetCurve2D->BasisCurve, asSingleEdge);//throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildOffset(basisWire, ifcOffsetCurve2D->Distance);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcOffsetCurve2D could not be built as a wire", ifcOffsetCurve2D);
				return wire;
			}
			TopoDS_Wire WireFactory::BuildWire(IIfcOffsetCurve3D^ ifcOffsetCurve3D, bool asSingleEdge)
			{
				TopoDS_Wire basisWire = BuildWire(ifcOffsetCurve3D->BasisCurve, asSingleEdge);//throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildOffset(basisWire, ifcOffsetCurve3D->Distance);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcOffsetCurve3D could not be built as a wire", ifcOffsetCurve3D);
				return wire;
			}
			TopoDS_Wire WireFactory::BuildWire(IIfcPolyline^ ifcPolyline, bool asSingleEdge)
			{
				if (asSingleEdge)
				{
					TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcPolyline); //throws exception
					TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IIfcPolyline could not be built as a wire", ifcPolyline);
					return wire;
				}
				else
				{
					if (2 == (int)ifcPolyline->Dim)
					{
						TColgp_Array1OfPnt2d points(1, ifcPolyline->Points->Count);
						int id = 0;

						for each (IIfcCartesianPoint ^ cp in ifcPolyline->Points)
						{
							points.SetValue(++id, GEOMETRY_FACTORY->BuildPoint2d(cp));
						}
						bool hasInfo;
						TopoDS_Wire wire = EXEC_NATIVE->BuildPolyline2d(points, ModelGeometryService->Precision, hasInfo);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IIfcPolyline could not be built as a wire", ifcPolyline);
						if (hasInfo)
							LogDebug(ifcPolyline, "Polyline has been corrected");
						return wire;
					}
					else
					{
						TColgp_Array1OfPnt points(1, ifcPolyline->Points->Count);
						int id = 0;

						for each (IIfcCartesianPoint ^ cp in ifcPolyline->Points)
						{
							points.SetValue(++id, GEOMETRY_FACTORY->BuildPoint3d(cp));
						}
						bool hasInfo;
						TopoDS_Wire wire = EXEC_NATIVE->BuildPolyline3d(points, ModelGeometryService->Precision, hasInfo);
						if (hasInfo)
							LogDebug(ifcPolyline, "Polyline has been corrected");
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IIfcPolyline could not be built as a wire", ifcPolyline);
						return wire;
					}
				}
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcRationalBSplineCurveWithKnots); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcLine could not be built as a wire", ifcRationalBSplineCurveWithKnots);
				return wire;
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcTrimmedCurve^ ifcTrimmedCurve, bool asSingleEdge)
			{
				TopoDS_Wire basisWire = BuildWire(ifcTrimmedCurve->BasisCurve, asSingleEdge);//throws exception
				bool isConic = (dynamic_cast<IIfcConic^>(ifcTrimmedCurve->BasisCurve) != nullptr);
				bool isLine = (dynamic_cast<IIfcLine^>(ifcTrimmedCurve->BasisCurve) != nullptr);
				bool isEllipse = (dynamic_cast<IIfcEllipse^>(ifcTrimmedCurve->BasisCurve) != nullptr);
				bool sense = ifcTrimmedCurve->SenseAgreement;
				//get the parametric values
				Xbim::Ifc4::Interfaces::IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

				bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == Xbim::Ifc4::Interfaces::IfcTrimmingPreference::CARTESIAN);

				double u1 = double::NegativeInfinity;
				double u2 = double::PositiveInfinity;
				IIfcCartesianPoint^ cp1 = nullptr;
				IIfcCartesianPoint^ cp2 = nullptr;
				gp_Pnt p1;
				gp_Pnt p2;
				for each (IIfcTrimmingSelect ^ trim in ifcTrimmedCurve->Trim1)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))cp1 = (IIfcCartesianPoint^)trim;
					else u1 = (double)(IfcParameterValue)trim; //its parametric	
				}
				for each (IIfcTrimmingSelect ^ trim in ifcTrimmedCurve->Trim2)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))cp2 = (IIfcCartesianPoint^)trim;
					else u2 = (double)(IfcParameterValue)trim; //its parametric	
				}

				if ((trim_cartesian && cp1 != nullptr && cp2 != nullptr) ||
					(cp1 != nullptr && cp2 != nullptr &&
						(double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)))) //we want cartesian and we have both or we don't have both parameters but have cartesians
				{
					p1 = GEOMETRY_FACTORY->BuildPoint3d(cp1);
					p2 = GEOMETRY_FACTORY->BuildPoint3d(cp2);
				}
				else if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //non-compliant
					throw RaiseGeometryFactoryException("Ifc Formal Proposition: TrimValuesConsistent. Either a single value is specified for Trim, or the two trimming values are of different type (point and parameter)", ifcTrimmedCurve);
				else //we prefer to use parameters but need to adjust
				{
					trim_cartesian = false;
					if (isConic)
					{
						u1 *= ModelGeometryService->RadianFactor; //correct to radians
						u2 *= ModelGeometryService->RadianFactor; //correct to radians
					}
				}

				if (Math::Abs(u1 - u2) < ModelGeometryService->Precision) //if the parameters are the same trimming will fail we cannot support periodic wires conic curve
				{
					throw RaiseGeometryFactoryException("Parametric Trim Points are equal and will result in an empty wire", ifcTrimmedCurve->BasisCurve);
				}
				else
				{
					//nb force the radian factor to 1.0 as we have already made the radian angle conversion for the pramameters
					TopoDS_Wire trimmedWire = EXEC_NATIVE->BuildTrimmedWire(basisWire, p1, p2, u1, u2, trim_cartesian, ifcTrimmedCurve->SenseAgreement, ModelGeometryService->MinimumGap, 1.0);
					if (trimmedWire.IsNull())
						throw RaiseGeometryFactoryException("IfcTrimmedCurve could not be built as a wire", ifcTrimmedCurve);
					return trimmedWire;
				}
			}


#pragma region Interface implementation



			IXWire^ WireFactory::BuildWire(array<IXPoint^>^ xPoints)
			{
				//validate
				if (xPoints == nullptr || xPoints->Length == 0)
					throw RaiseGeometryFactoryException("Points has zero length");
				TColgp_Array1OfPnt points(1, xPoints->Length);
				int id = 0;

				for each (IXPoint ^ cp in xPoints)
				{
					points.SetValue(++id, GEOMETRY_FACTORY->BuildPoint3d(cp));
				}
				bool hasInfo;
				TopoDS_Wire wire = EXEC_NATIVE->BuildPolyline3d(points,/* -1, -1,*/ ModelGeometryService->Precision, hasInfo);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Resulting wire is empty");
				if (hasInfo)
					LogInformation("Points array has been corrected");
				if (OccHandle().IsClosed(wire, ModelGeometryService->Precision))
					wire.Closed(true);
				return gcnew XWire(wire);
			}


			IXWire^ WireFactory::Build(IIfcCurve^ ifcCurve)
			{
				TopoDS_Wire wire = BuildWire(ifcCurve, false); //asume all compund edges are implemented as segments, not bsplines
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Resulting wire is empty", ifcCurve);
				return gcnew XWire(wire);
			}

			IXWire^ WireFactory::Build(IIfcProfileDef^ ifcProfileDef)
			{
				return PROFILE_FACTORY->BuildWire(ifcProfileDef);
			}




#pragma endregion


			TopoDS_Wire WireFactory::BuildWire(IIfcCompositeCurve^ ifcCompositeCurve, bool asSingleEdge)
			{
				if (asSingleEdge)
				{
					TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcCompositeCurve); //throws exception
					TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);
					return wire;
				}
				else
				{
					if ((int)ifcCompositeCurve->Dim == 2)
					{
						TColGeom2d_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildCompositeCurveSegments2d(ifcCompositeCurve, segments);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);
						return wire;
					}
					else
					{
						TColGeom_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildCompositeCurveSegments3d(ifcCompositeCurve, segments);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
						{
#ifdef _DEBUG

							System::Text::StringBuilder msg("Error building Composite curve\n");
							gp_Pnt lastPnt;
							for (auto&& seg : segments)
							{
								auto start = seg->StartPoint();
								auto end = seg->EndPoint();
								msg.AppendFormat("({0},{1},{2}) -> ({3},{4},{5}) \t\tGAP {6}\n", start.X(), start.Y(), start.Z(), end.X(), end.Y(), end.Z(), Math::Round(lastPnt.Distance(start), 3));
								lastPnt = end;

							}
							LogDebug(ifcCompositeCurve, nullptr, msg.ToString());
#endif // _DEBUG

							throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);
						}
						return wire;
					}
				}
			}


			TopoDS_Wire WireFactory::BuildWire(IIfcCompositeCurve^ ifcCompositeCurve, System::Nullable<double> startParam, System::Nullable<double> endParam)
			{

				TColGeom_SequenceOfBoundedCurve segments;
				double occStart = 0;
				double occEnd = 0;
				double totCurveLen = 0;
				double firstParameterizedLength = 0;
				double startPar = 0;
				double endPar = double::PositiveInfinity;

				if (startParam.HasValue && !double::IsNaN(startParam.Value))
					startPar = startParam.Value;
				if (endParam.HasValue && !double::IsNaN(endParam.Value))
					endPar = endParam.Value;

				int i = 0;
				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{

					if (startPar <= 0 && endPar <= 0) // terminating cuz we don't need to build segments any further
						continue;

					IIfcReparametrisedCompositeCurveSegment^ reparameterisedSegment = dynamic_cast<IIfcReparametrisedCompositeCurveSegment^>(segment);
					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (!CURVE_FACTORY->IsBoundedCurve(segment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");

					double segmentParameterizedLength = SegmentLength(segment);
					double length = 0.0;

					if (i == 0)
						firstParameterizedLength = segmentParameterizedLength;

					//if the segment is a polyline or an indexedpolycurve we need to add in the individual edge
					auto polylineSegment = dynamic_cast<IIfcPolyline^>(segment->ParentCurve);
					auto indexPolyCurveSegment = dynamic_cast<IIfcIndexedPolyCurve^>(segment->ParentCurve);
					if (polylineSegment != nullptr)
					{
						CURVE_FACTORY->BuildPolylineSegments3d(polylineSegment, segments, length);
						totCurveLen += length;
						segmentParameterizedLength = polylineSegment->Points->Count - 1;
					}
					else if (indexPolyCurveSegment != nullptr)
					{
						double pLength;
						CURVE_FACTORY->BuildIndexPolyCurveSegments3d(indexPolyCurveSegment, segments, length, pLength);
						totCurveLen += length;
						segmentParameterizedLength = pLength;
					}
					else
					{
						Handle(Geom_Curve) hSegment = CURVE_FACTORY->BuildCompositeCurveSegment3d(segment->ParentCurve, segment->SameSense);

						if (hSegment.IsNull())
							continue;//this will throw an exception if badly defined, a zero length segment (IsNull) is tolerated

						Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(hSegment);
						if (boundedCurve.IsNull())
							throw RaiseGeometryFactoryException("Compound curve segments must be bounded curves", segment);


						GeomAdaptor_Curve adaptor(hSegment);
						Standard_Real f = hSegment->FirstParameter();
						Standard_Real l = hSegment->LastParameter();
						length = GCPnts_AbscissaPoint::Length(adaptor, f, l);
						totCurveLen += length;
						segments.Append(boundedCurve);
					}


					if (startPar > 0)
					{
						double ratio = Math::Min(startPar / segmentParameterizedLength, 1.0);
						startPar -= ratio * segmentParameterizedLength;
						occStart += ratio * length;
					}

					if (endPar > 0)
					{
						if (endPar <= firstParameterizedLength && endParam.HasValue && endParam.Value == 1 && startParam.HasValue && startParam.Value == 0)
						{
							occEnd += length;
						}
						else
						{
							double ratio = Math::Min(endPar / segmentParameterizedLength, 1.0);
							endPar -= ratio * segmentParameterizedLength;
							occEnd += ratio * length;
						}
					}

					i++;

				}

				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);

				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);

				if (Math::Abs(occStart - 0.0) < ModelGeometryService->Precision && Math::Abs(occEnd - totCurveLen) < ModelGeometryService->Precision)
					return wire;

				wire = EXEC_NATIVE->BuildTrimmedWire(wire, occStart, occEnd, true, ModelGeometryService->Precision, ModelGeometryService->RadianFactor);

				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcCompositeCurve could not be trimmed", ifcCompositeCurve);

				return wire;
			}


			double WireFactory::SegmentLength(IIfcCompositeCurveSegment^ segment)
			{
				IIfcLine^ line = dynamic_cast<IIfcLine^>(segment->ParentCurve);
				IIfcTrimmedCurve^ trimmedCurve = dynamic_cast<IIfcTrimmedCurve^>(segment->ParentCurve);
				if (line != nullptr)
				{
					return 1;
				}
				else if (trimmedCurve != nullptr)
				{
					try {
						IIfcTrimmedCurve^ tc = dynamic_cast<IIfcTrimmedCurve^>(segment->ParentCurve);
						double valTrim1 = 0;
						double valTrim2 = 1;
						// search for parameter values
						Xbim::Ifc4::MeasureResource::IfcParameterValue^ t;

						for (int i = 0; i < tc->Trim1->Count; i++)
						{
							t = dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(tc->Trim1[i]);
							if (t && t->Value)
							{
								valTrim1 = (double)t->Value;
								break;
							}
						}
						for (int i = 0; i < tc->Trim2->Count; i++)
						{
							t = dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(tc->Trim2[i]);
							if (t && t->Value)
							{
								valTrim2 = (double)t->Value;
								break;
							}
						}
						double ret = valTrim2 - valTrim1;

						if (ret < 0 && (dynamic_cast<IIfcConic^>(tc->BasisCurve) != nullptr)) //params will be periodic so take the abs length
							ret = Math::Abs(ret);
						if (ret < 0)
						{
							return 1;
						}
						return ret;
					}
					catch (Exception^) {
						return 1;
					}
				}
				return 1;
			}


			struct WireFactoryNativeStatics
			{
				static std::shared_mutex execNativeMutex;
			};

			std::shared_mutex WireFactoryNativeStatics::execNativeMutex;


			TopoDS_Wire WireFactory::BuildWire(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, bool asSingleEdge)
			{
				if (asSingleEdge)
				{
					TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcIndexedPolyCurve); //throws exception
					TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
					return wire;
				}
				else
				{
					if (2 == (int)ifcIndexedPolyCurve->Dim)
					{
						std::lock_guard<std::shared_mutex> lock(WireFactoryNativeStatics::execNativeMutex);
						TColGeom2d_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildIndexPolyCurveSegments2d(ifcIndexedPolyCurve, segments);
						if (segments.Length() == 0)
						{
							// segments is empty
							throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
						}
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
						return wire;
					}
					else
					{
						TColGeom_SequenceOfBoundedCurve segments;
						double length;
						double pLength;
						CURVE_FACTORY->BuildIndexPolyCurveSegments3d(ifcIndexedPolyCurve, segments, length, pLength);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
						return wire;
					}
				}
			}


			TopoDS_Wire WireFactory::BuildWire(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, System::Nullable<double> startParam, System::Nullable<double> endParam)
			{
				TColGeom_SequenceOfBoundedCurve segments;

				double totalLength = 0.0;
				double occStart = 0;
				double occEnd = 0;
				double startPar = 0;
				double endPar = double::PositiveInfinity;

				if (startParam.HasValue && !double::IsNaN(startParam.Value))
					startPar = startParam.Value;
				if (endParam.HasValue && !double::IsNaN(endParam.Value))
					endPar = endParam.Value;

				IIfcCartesianPointList3D^ pointList3D = dynamic_cast<IIfcCartesianPointList3D^>(ifcIndexedPolyCurve->Points);
				if (pointList3D == nullptr)
					throw RaiseGeometryFactoryException("IIfcIndexedPolyCurve point list is not 3D", ifcIndexedPolyCurve->Points);


				int pointCount = pointList3D->CoordList->Count;
				TColgp_Array1OfPnt poles(1, pointCount);
				int i = 1;
				for each (IItemSet<Ifc4::MeasureResource::IfcLengthMeasure> ^ coll in pointList3D->CoordList)
				{
					IEnumerator<Ifc4::MeasureResource::IfcLengthMeasure>^ enumer = coll->GetEnumerator();
					enumer->MoveNext();
					gp_Pnt p;
					p.SetX((double)enumer->Current);
					enumer->MoveNext();
					p.SetY((double)enumer->Current);
					enumer->MoveNext();
					p.SetZ((double)enumer->Current);
					poles.SetValue(i, p);
					i++;
				}

				if (ifcIndexedPolyCurve->Segments != nullptr && Enumerable::Any(ifcIndexedPolyCurve->Segments))
				{
					for each (IIfcSegmentIndexSelect ^ segment in  ifcIndexedPolyCurve->Segments)
					{
						if (startPar <= 0 && endPar <= 0)
							continue;

						Ifc4::GeometryResource::IfcArcIndex^ arcIndex = dynamic_cast<Ifc4::GeometryResource::IfcArcIndex^>(segment);
						Ifc4::GeometryResource::IfcLineIndex^ lineIndex = dynamic_cast<Ifc4::GeometryResource::IfcLineIndex^>(segment);
						double segmentParameterizedLength = 0;
						double length = 0;

						if (arcIndex != nullptr)
						{

							List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)arcIndex->Value;
							if (indices->Count != 3)
								throw RaiseGeometryFactoryException("There should be three indices in an arc index segment", ifcIndexedPolyCurve);
							gp_Pnt start = poles.Value((int)indices[0]);
							gp_Pnt mid = poles.Value((int)indices[1]);
							gp_Pnt end = poles.Value((int)indices[2]);
							Handle(Geom_Circle) circle = CURVE_FACTORY->Ptr()->BuildCircle3d(start, mid, end);
							if (!circle.IsNull()) //it is a valid arc
							{
								Handle(Geom_TrimmedCurve) arcSegment = CURVE_FACTORY->Ptr()->BuildTrimmedCurve3d(circle, start, end, ModelGeometryService->MinimumGap);
								if (arcSegment.IsNull())
									throw RaiseGeometryFactoryException("Failed to trim Arc Index segment", ifcIndexedPolyCurve);

								Standard_Real f = arcSegment->FirstParameter();
								Standard_Real l = arcSegment->LastParameter();
								segmentParameterizedLength = Abs(l - f);
								length = circle->Radius() * segmentParameterizedLength;
								totalLength += length;
								segments.Append(arcSegment);
							}
							else //most likley the three points are in a line it should be treated as a polyline segment according the the docs
							{
								LogInformation(ifcIndexedPolyCurve, "An ArcIndex of an IfcIndexedPolyCurve has been handled as a LineIndex");
								Handle(Geom_TrimmedCurve) lineSegment = CURVE_FACTORY->Ptr()->BuildTrimmedLine3d(start, end);
								if (lineSegment.IsNull())
									throw RaiseGeometryFactoryException("A LineIndex of an IfcIndexedPolyCurve could not be built", ifcIndexedPolyCurve);
								length = start.Distance(end);
								totalLength += length;
								segmentParameterizedLength = 1;
								segments.Append(lineSegment);

							}
						}
						else if (lineIndex != nullptr)
						{
							List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)lineIndex->Value;

							if (indices->Count < 2)
								throw RaiseGeometryFactoryException("There should be at least two indices in a line index segment", ifcIndexedPolyCurve);

							for (Standard_Integer p = 1; p <= indices->Count - 1; p++)
							{
								gp_Pnt p1 = poles.Value((int)indices[p - 1]);
								gp_Pnt p2 = poles.Value((int)indices[p]);

								Handle(Geom_TrimmedCurve) lineSegment = CURVE_FACTORY->Ptr()->BuildTrimmedLine3d(p1, p2);

								if (lineSegment.IsNull())
									throw RaiseGeometryFactoryException("A line index segment was invalid", ifcIndexedPolyCurve);
								length += p1.Distance(p2);
								segments.Append(lineSegment);
							}

							segmentParameterizedLength = indices->Count - 1;
							totalLength += length;

						}


						if (startPar > 0)
						{
							double ratio = Math::Min(startPar / segmentParameterizedLength, 1.0);
							startPar -= ratio * segmentParameterizedLength;
							occStart += ratio * length;
						}

						if (endPar > 0)
						{
							double ratio = Math::Min(endPar / segmentParameterizedLength, 1.0);
							endPar -= ratio * segmentParameterizedLength;
							occEnd += ratio * length;
						}

					}
				}
				else
				{
					// To be compliant with:
					// "In the case that the list of Segments is not provided, all points in the IfcCartesianPointList are connected by straight line segments in the order they appear in the IfcCartesianPointList."
					// http://www.buildingsmart-tech.org/ifc/IFC4/Add1/html/schema/ifcgeometryresource/lexical/ifcindexedpolycurve.htm
					for (Standard_Integer p = 1; p < pointCount; p++)
					{
						gp_Pnt p1 = poles.Value(p);
						gp_Pnt p2 = poles.Value(p + 1);

						Handle(Geom_TrimmedCurve) lineSegment = CURVE_FACTORY->Ptr()->BuildTrimmedLine3d(p1, p2);

						if (lineSegment.IsNull())
							throw RaiseGeometryFactoryException("A line index segment was invalid", ifcIndexedPolyCurve);

						totalLength += p1.Distance(p2);

						segments.Append(lineSegment);
					}

					occStart = startPar;
					occEnd = endPar;
				}

				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);

				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);

				if (Math::Abs(occStart - 0.0) < ModelGeometryService->Precision && Math::Abs(occEnd - totalLength) < ModelGeometryService->Precision)
					return wire;

				wire = EXEC_NATIVE->BuildTrimmedWire(wire, occStart, occEnd, true, ModelGeometryService->Precision, ModelGeometryService->RadianFactor);

				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be trimmed", ifcIndexedPolyCurve);

				return wire;
			}


			TopoDS_Wire WireFactory::BuildDirectrixWire(IIfcCurve^ ifcCurve, double startParam, double endParam)
			{

				TopoDS_Wire wire;

				if (dynamic_cast<IIfcPolyline^>(ifcCurve) &&
					startParam == 0. &&
					endParam == 1. &&
					BIM_WORKAROUNDS->ShouldApplyPolylineTrimLengthOneForEntireLine()) //consider work around for incorrectly set trims
				{
					endParam = double::NaN; //set to max
					LogDebug(ifcCurve, "Polyline trim (0:1) does not comply with schema. {0}. It has been expanded to the entire length of the Polyline", ModelGeometryService->Model->Header->FileName->OriginatingSystem);
				}


				if (dynamic_cast<IIfcCompositeCurve^>(ifcCurve))
				{
					IIfcCompositeCurve^ curve = (IIfcCompositeCurve^)(ifcCurve);
					wire = BuildWire(curve, startParam, endParam);
					return wire;
				}
				else if (dynamic_cast<IIfcIndexedPolyCurve^>(ifcCurve))
				{
					IIfcIndexedPolyCurve^ curve = (IIfcIndexedPolyCurve^)(ifcCurve);
					wire = BuildWire(curve, startParam, endParam);
					return wire;
				}
				else {
					wire = BuildWire(ifcCurve, false);
				}


				if (double::IsNaN(startParam) && double::IsNaN(endParam))
					return wire;

				//nb, at this point we have made no attempt to convert the params to radians, the BuildTrimmedWire will do this if we pass the radian conversion factor through
				TopoDS_Wire directrix = EXEC_NATIVE->BuildTrimmedWire(wire, startParam, endParam, true, ModelGeometryService->Precision, ModelGeometryService->RadianFactor);

				if (directrix.IsNull())
					throw RaiseGeometryFactoryException("Directrix could not be built", ifcCurve);

				return directrix;
			}


			bool WireFactory::Fillet(const TopoDS_Wire& directrix, TopoDS_Wire& filletedDirectrix, double filletRadius)
			{
				filletedDirectrix = EXEC_NATIVE->Fillet(directrix, filletRadius, _modelService->Precision);
				return !filletedDirectrix.IsNull();
			}


		}
	}
}