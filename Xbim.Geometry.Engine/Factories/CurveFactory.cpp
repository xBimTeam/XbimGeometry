#include "CurveFactory.h"
#include "GeometryFactory.h"
#include "SurfaceFactory.h"

#include <GeomLib.hxx>
#include <GeomLib_Tool.hxx>
#include <math.h>
#include <optional>

#include "../BRep/XLine.h"
#include "../BRep/XSpiral.h"
#include "../BRep/XLine2d.h"
#include "../BRep/XCircle.h"
#include "../BRep/XCircle2d.h"
#include "../BRep/XEllipse.h"
#include "../BRep/XEllipse2d.h"
#include "../BRep/XTrimmedCurve.h"
#include "../BRep/XTrimmedCurve2d.h"
#include "../BRep/XBSplineCurve.h"
#include "../BRep/OccExtensions/Curves/Segments/Geom2d_Spiral.h"
#include "../BRep/OccExtensions/Curves/Segments/Geom2d_Polynomial.h"

#include "TColgp_Array1OfPnt2d.hxx"
#include "TColStd_Array1OfReal.hxx"
#include "TColStd_Array1OfInteger.hxx"

#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <TColGeom_SequenceOfBoundedCurve.hxx>
#include <NCollection_Vector.hxx>
#include "../BRep/OccExtensions/KeyedPnt.h"
#include "../BRep/OccExtensions/KeyedPnt2d.h"
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <Geom2dAPI_PointsToBSpline.hxx>
#include <ShapeExtend_ComplexCurve.hxx>


/*
The approach of the curve factory is to build all curves as IXCurve using the build method.
This will ensure correct dimensionality of the curves is maintained
Most curve types can have a 2D or a 3D variant, Ifc hides this in the Dim method.
Any 3D shape can be built from a 2D definition with the Z coordinate set to Zero
It is nor permitted to create 2D shapes from 3D definitions and an exception is thrown.
Managed code is used to navigate the definitions and provide the framework for the unmanaged code to build the native curves
All validation is done is managed code
All operations where an OCC exception will be thrown are implemented in unmanaged code. (NCurveFactory)
These exceptions are caught and logged in the managed code
Unmanaged build methods return a null handle to the specified geometry type when a standard failure condition has been thrown
*/
using namespace System;
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Ifc4::MeasureResource;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Common::Metadata;
using namespace Xbim::Common;
using namespace System::Linq;

using namespace System::Collections::Generic;
using namespace Xbim::Common::Collections;
using namespace Xbim::Ifc4::Interfaces;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{

			IXCurve^ CurveFactory::Build(IIfcCurve^ curve)
			{
				XCurveType curveType;
				int dim = (int)curve->Dim;

				if (dim == 2)
				{

					Handle(Geom2d_Curve) hCurve2d = BuildCurve2d(curve, curveType); //this will throw an exception if it fails		
					return BuildXCurve(hCurve2d, curveType);
				}
				else
				{
					Handle(Geom_Curve) hCurve = BuildCurve(curve, curveType); //this will throw an exception if it fails				
					return BuildXCurve(hCurve, curveType);
				}
			}


			IXCurve^ CurveFactory::BuildDirectrix(IIfcCurve^ curve, Nullable<double> startParam, Nullable<double> endParam)
			{
				double start, end;
				if (startParam.HasValue) start = startParam.Value; else start = -1;
				if (endParam.HasValue) end = endParam.Value; else  end = -1;
				return BuildDirectrix(curve, start, end);
			}

			IXCurve^ CurveFactory::BuildSpiral(Ifc4x3::GeometryResource::IfcSpiral^ curve, double startParam, double endParam)
			{
				Geom2d_Spiral::IntegrationSteps = (int)((endParam - startParam) / _modelService->OneMeter);
				XCurveType spiralType;
				Handle(Geom2d_Curve) spiralCurve = EXEC_NATIVE->MoveBoundedCurveToOrigin(BuildBoundedSpiral(curve, startParam, endParam, spiralType));
				return gcnew XCurve2d(spiralCurve, spiralType);
			}

			IXCurve^ CurveFactory::BuildPolynomialCurve2d(Ifc4x3::GeometryResource::IfcPolynomialCurve^ curve, double startParam, double endParam) 
			{
				Geom2d_Spiral::IntegrationSteps = (int)((endParam - startParam) / _modelService->OneMeter);
				return gcnew XCurve2d(EXEC_NATIVE->MoveBoundedCurveToOrigin(BuildBoundedPolynomialCurve(curve, startParam, endParam)), XCurveType::IfcPolynomialCurve);
			}

			IXCurve^ CurveFactory::BuildXCurve(Handle(Geom_Curve) curve, XCurveType curveType)
			{
				switch (curveType)
				{
					case XCurveType::IfcCircle:
						return gcnew XCircle(Handle(Geom_Circle)::DownCast(curve));
					case XCurveType::IfcCompositeCurve:
						return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(curve));
					case XCurveType::IfcEllipse:
						return gcnew XEllipse(Handle(Geom_Ellipse)::DownCast(curve));
					case XCurveType::IfcLine:
						return gcnew Xbim::Geometry::BRep::XLine(Handle(Geom_LineWithMagnitude)::DownCast(curve));
					case XCurveType::IfcTrimmedCurve:
						return gcnew XTrimmedCurve(Handle(Geom_TrimmedCurve)::DownCast(curve));
					case XCurveType::IfcIndexedPolyCurve:
					case XCurveType::IfcCompositeCurveOnSurface:
					case XCurveType::IfcBSplineCurveWithKnots:
					case XCurveType::IfcOffsetCurve2D:
					case XCurveType::IfcPcurve:
					case XCurveType::IfcPolyline:
					case XCurveType::IfcRationalBSplineCurveWithKnots:
					case XCurveType::IfcSurfaceCurve:
					default:
						return gcnew XCurve(curve, curveType);
				}
				throw RaiseGeometryFactoryException("Unsupported curve type");
			}

			IXCurve^ CurveFactory::BuildXCurve(Handle(Geom2d_Curve) curve, XCurveType curveType)
			{
				switch (curveType)
				{
					case XCurveType::IfcCircle:
						return gcnew XCircle2d(Handle(Geom2d_Circle)::DownCast(curve));
					case XCurveType::IfcEllipse:
						return gcnew XEllipse2d(Handle(Geom2d_Ellipse)::DownCast(curve));
					case XCurveType::IfcLine:
						return gcnew XLine2d(Handle(Geom2d_LineWithMagnitude)::DownCast(curve));
					case XCurveType::IfcTrimmedCurve:
						return gcnew XTrimmedCurve2d(Handle(Geom2d_TrimmedCurve)::DownCast(curve));
					case XCurveType::IfcIndexedPolyCurve:
					case XCurveType::IfcCompositeCurve:
					case XCurveType::IfcCompositeCurveOnSurface:
					case XCurveType::IfcBSplineCurveWithKnots:
					case XCurveType::IfcOffsetCurve2D:
					case XCurveType::IfcPcurve:
					case XCurveType::IfcPolyline:
					case XCurveType::IfcRationalBSplineCurveWithKnots:
					case XCurveType::IfcSurfaceCurve:
					
					default:
						return gcnew XCurve2d(curve, curveType);
				}
				throw RaiseGeometryFactoryException("Unsupported 2d curve type");
			}

			IXCurve^ CurveFactory::BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam)
			{
				XCurveType curveType;
				Handle(Geom_Curve) directix = BuildDirectrix(curve, startParam, endParam, curveType);
				switch (curveType)
				{
					case XCurveType::IfcBSplineCurveWithKnots:
						return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(directix));
					case XCurveType::IfcCircle:
						return gcnew XCircle(Handle(Geom_Circle)::DownCast(directix));
					case XCurveType::IfcCompositeCurve:
						return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(directix));
						/*case XCurveType::CompositeCurveOnSurface:
							return Build3d(dynamic_cast<IIfcCompositeCurveOnSurface^>(curve));*/
					case XCurveType::IfcEllipse:
						return gcnew XEllipse(Handle(Geom_Ellipse)::DownCast(directix));
					case XCurveType::IfcIndexedPolyCurve:
						return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(directix));
					case XCurveType::IfcLine:
						return gcnew Xbim::Geometry::BRep::XLine(Handle(Geom_LineWithMagnitude)::DownCast(directix));

					case XCurveType::IfcOffsetCurve3D:	//need to enhance interface wrappers for these				
					case XCurveType::IfcPolyline:
						return gcnew Xbim::Geometry::BRep::XLine(Handle(Geom_LineWithMagnitude)::DownCast(directix));
						return gcnew XCurve(directix, curveType);
					case XCurveType::IfcRationalBSplineCurveWithKnots:
						return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(directix));
						/*	case XCurveType::SurfaceCurve:
								return Build3d(dynamic_cast<IIfcSurfaceCurve^>(directix));*/
					case XCurveType::IfcTrimmedCurve:
						return gcnew XTrimmedCurve(Handle(Geom_TrimmedCurve)::DownCast(directix));
						break;
					default:
						throw RaiseGeometryFactoryException("Unsupported curve type");
				}
				throw RaiseGeometryFactoryException("Unsupported curve type");
			}


#pragma region 2d Curve builders


			void CurveFactory::BuildIndexPolyCurveSegments2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, TColGeom2d_SequenceOfBoundedCurve& segments)
			{

				IIfcCartesianPointList2D^ pointList2D = dynamic_cast<IIfcCartesianPointList2D^>(ifcIndexedPolyCurve->Points);
				if (pointList2D == nullptr)
					throw RaiseGeometryFactoryException("IIfcIndexedPolyCurve point list is not 2D", ifcIndexedPolyCurve->Points);


				//get a index of all the points
				int pointCount = pointList2D->CoordList->Count;
				TColgp_Array1OfPnt2d poles(1, pointCount);
				int i = 1;
				for each (IItemSet<Ifc4::MeasureResource::IfcLengthMeasure> ^ coll in pointList2D->CoordList)
				{
					IEnumerator<Ifc4::MeasureResource::IfcLengthMeasure>^ enumer = coll->GetEnumerator();
					enumer->MoveNext();
					gp_Pnt2d p;
					p.SetX((double)enumer->Current);
					enumer->MoveNext();
					p.SetY((double)enumer->Current);
					poles.SetValue(i, p);
					i++;
				}

				if (ifcIndexedPolyCurve->Segments != nullptr && Enumerable::Any(ifcIndexedPolyCurve->Segments))
				{
					for each (IIfcSegmentIndexSelect ^ segment in  ifcIndexedPolyCurve->Segments)
					{
						Ifc4::GeometryResource::IfcArcIndex^ arcIndex = dynamic_cast<Ifc4::GeometryResource::IfcArcIndex^>(segment);
						Ifc4::GeometryResource::IfcLineIndex^ lineIndex = dynamic_cast<Ifc4::GeometryResource::IfcLineIndex^>(segment);
						if (arcIndex != nullptr)
						{

							List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)arcIndex->Value;
							if (indices->Count != 3)
								throw RaiseGeometryFactoryException("There should be three indices in an arc index segment", ifcIndexedPolyCurve);
							gp_Pnt2d start = poles.Value((int)indices[0]);
							gp_Pnt2d mid = poles.Value((int)indices[1]);
							gp_Pnt2d end = poles.Value((int)indices[2]);
							Handle(Geom2d_Circle) circle = OccHandle().BuildCircle2d(start, mid, end);
							if (!circle.IsNull()) //it is a valid arc
							{
								Handle(Geom2d_TrimmedCurve) arcSegment = OccHandle().BuildTrimmedCurve2d(circle, start, end, ModelGeometryService->MinimumGap);
								if (arcSegment.IsNull())
									throw RaiseGeometryFactoryException("Failed to trim Arc Index segment", ifcIndexedPolyCurve);
								segments.Append(arcSegment);
							}
							else //most likley the three points are in a line it should be treated as a polyline segment according the the docs
							{
								LogInformation(ifcIndexedPolyCurve, "An ArcIndex of an IfcIndexedPolyCurve has been handled as a LineIndex");
								Handle(Geom2d_TrimmedCurve) lineSegment = OccHandle().BuildTrimmedLine2d(start, end);
								if (lineSegment.IsNull())
									throw RaiseGeometryFactoryException("A LineIndex of an IfcIndexedPolyCurve could not be built", ifcIndexedPolyCurve);
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
								Handle(Geom2d_TrimmedCurve) lineSegment = OccHandle().BuildTrimmedLine2d(poles.Value((int)indices[p - 1]), poles.Value((int)indices[p]));
								if (lineSegment.IsNull())
									throw RaiseGeometryFactoryException("A line index segment was invalid", ifcIndexedPolyCurve);
								segments.Append(lineSegment);
							}
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
						Handle(Geom2d_TrimmedCurve) lineSegment = OccHandle().BuildTrimmedLine2d(poles.Value(p), poles.Value(p + 1));
						if (lineSegment.IsNull())
							throw RaiseGeometryFactoryException("A line index segment was invalid", ifcIndexedPolyCurve);
						segments.Append(lineSegment);
					}
				}
			}
			

			void CurveFactory::BuildCompositeCurveSegments2d(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom2d_SequenceOfBoundedCurve& segments)
			{
				segments.Clear();

				int lastLabel = -1;
				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{
					//a bug in some archicad models writes each segment out twice, ignore if we get the same segment as the previous one, using the IFC label, this never makes sense
					if (lastLabel == segment->EntityLabel)
						continue;
					lastLabel = segment->EntityLabel;
					IIfcReparametrisedCompositeCurveSegment^ reparameterisedSegment = dynamic_cast<IIfcReparametrisedCompositeCurveSegment^>(segment);
					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (!IsBoundedCurve(segment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");

					//if the segment is a polyline or an indexedpolycurve we need to add in the individual edge
					auto polylineSegment = dynamic_cast<IIfcPolyline^>(segment->ParentCurve);
					auto indexPolyCurveSegment = dynamic_cast<IIfcIndexedPolyCurve^>(segment->ParentCurve);
					if (polylineSegment != nullptr)
					{
						BuildPolylineSegments2d(polylineSegment, segments);
					}
					else if (indexPolyCurveSegment != nullptr)
					{
						BuildIndexPolyCurveSegments2d(indexPolyCurveSegment, segments);
					}
					else
					{
						Handle(Geom2d_Curve) hSegment = BuildCompositeCurveSegment2d(segment->ParentCurve, segment->SameSense);
						if (hSegment.IsNull()) continue; //this will throw an exception if badly defined, a zero length segment (IsNull) is tolerated
						Handle(Geom2d_BoundedCurve) boundedCurve = Handle(Geom2d_BoundedCurve)::DownCast(hSegment);
						if (boundedCurve.IsNull())
							throw RaiseGeometryFactoryException("Compound curve segments must be bounded curves", segment);
						segments.Append(boundedCurve);
					}
				}
			}

			Handle(Geom2d_Curve) CurveFactory::BuildAxis2d(IIfcGridAxis^ axis)
			{
				if (2 != (int)axis->AxisCurve->Dim)
					RaiseGeometryFactoryException("Axis must have a 2d curve");
				XCurveType curveType;
				Handle(Geom2d_Curve) curve2d = BuildCurve2d(axis->AxisCurve, curveType); //throws exception
				if (!axis->SameSense) curve2d->Reverse();
				return curve2d;
			}

			int CurveFactory::Intersections(const Handle(Geom2d_Curve)& c1, const Handle(Geom2d_Curve)& c2, TColgp_Array1OfPnt2d& intersections)
			{
				int intersectCnt = EXEC_NATIVE->Intersections(c1, c2, intersections, ModelGeometryService->MinimumGap);
				if (intersectCnt < 0)
					RaiseGeometryFactoryException("Calculation of Curve intersections failed");
				return intersectCnt;
			}



#pragma endregion


#pragma region Build 3d Curves

			Handle(Geom_Curve) CurveFactory::BuildCurve(IIfcCurve^ curve)
			{
				XCurveType curveType;
				return BuildCurve(curve, curveType);
			}

			Handle(Geom_Curve) CurveFactory::BuildCurve(IIfcCurve^ curve, XCurveType% curveType)
			{
				if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
					throw RaiseGeometryFactoryException("Unsupported curve type.", curve);

				switch (curveType)
				{
					case XCurveType::IfcBSplineCurveWithKnots:
						return BuildCurve(static_cast<IIfcBSplineCurveWithKnots^>(curve));
					case XCurveType::IfcCircle:
						return BuildCurve(static_cast<IIfcCircle^>(curve));
					case XCurveType::IfcCompositeCurve:
					{
						Ifc4x3::GeometryResource::IfcCompositeCurve^ ifc4x3Curve = dynamic_cast<Ifc4x3::GeometryResource::IfcCompositeCurve^>(curve);
						if(ifc4x3Curve != nullptr)
							return BuildCurve(ifc4x3Curve);

						return BuildCurve(static_cast<IIfcCompositeCurve^>(curve));
					}
					case XCurveType::IfcCompositeCurveOnSurface:
						return BuildCurve(static_cast<IIfcCompositeCurveOnSurface^>(curve));
					case XCurveType::IfcEllipse:
						return BuildCurve(static_cast<IIfcEllipse^>(curve));
					case XCurveType::IfcIndexedPolyCurve:
						return BuildCurve(static_cast<IIfcIndexedPolyCurve^>(curve));
					case XCurveType::IfcLine:
						return BuildCurve(static_cast<IIfcLine^>(curve));
					case XCurveType::IfcOffsetCurve2D:
						return BuildCurve(static_cast<IIfcOffsetCurve2D^>(curve));
					case XCurveType::IfcOffsetCurve3D:
						return BuildCurve(static_cast<IIfcOffsetCurve3D^>(curve));
					case XCurveType::IfcPcurve:
						return BuildCurve(static_cast<IIfcPcurve^>(curve));
					case XCurveType::IfcPolyline:
						return BuildCurve(static_cast<IIfcPolyline^>(curve));
					case XCurveType::IfcRationalBSplineCurveWithKnots:
						return BuildCurve(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::IfcSurfaceCurve:
						return BuildCurve(static_cast<IIfcSurfaceCurve^>(curve));
					case XCurveType::IfcTrimmedCurve:
						return BuildCurve(static_cast<IIfcTrimmedCurve^>(curve));
					case XCurveType::IfcGradientCurve:
						return BuildCurve(static_cast<Ifc4x3::GeometryResource::IfcGradientCurve^>(curve));
					case XCurveType::IfcSegmentedReferenceCurve:
						return BuildCurve(static_cast<Ifc4x3::GeometryResource::IfcSegmentedReferenceCurve^>(curve));
					default:
						throw RaiseGeometryFactoryException("Curve type not implemented.", curve);
				}
				throw;
			}
			Handle(Geom_GradientCurve) CurveFactory::BuildCurve(Ifc4x3::GeometryResource::IfcGradientCurve^ ifcGradientCurve)
			{
				std::optional<Handle(Standard_Transient)> cached = GetCache()->Get(ifcGradientCurve->EntityLabel);
				if (cached.has_value()) {
					return Handle(Geom_GradientCurve)::DownCast(cached.value());
				}

				// The base curve is the horizontal projection
				XCurveType curveType;
				Handle(Geom2d_Curve) horizontalProjection = BuildCurve2d(ifcGradientCurve->BaseCurve, curveType);

				if (horizontalProjection.IsNull()) {
					throw RaiseGeometryFactoryException("IfcGradientCurve BaseCurve could not be built", ifcGradientCurve);
				}

				// Building heigth function from the curve Segments
				for (int i = 0; i < ifcGradientCurve->NSegments; i++)
				{
					Ifc4x3::GeometryResource::IfcCurveSegment^ curveSegment = dynamic_cast<Ifc4x3::GeometryResource::IfcCurveSegment^>(ifcGradientCurve->Segments[i]);
					if (curveSegment == nullptr)
						throw RaiseGeometryFactoryException("IfcGradientCurve Segments should be of type IfcCurveSegment", ifcGradientCurve);
				}

				std::vector<Handle(Geom2d_Curve)> heightFunctionCurves = ProcessSegments(ifcGradientCurve);
			
				bool hasEndPoint = false;
				gp_Pnt2d endPoint;

				if (ifcGradientCurve->EndPoint != nullptr)
				{
					IIfcAxis2Placement2D^ axis2Placement = dynamic_cast<IIfcAxis2Placement2D^>(ifcGradientCurve->EndPoint);

					TopLoc_Location location;
					if (axis2Placement) {
						hasEndPoint = GEOMETRY_FACTORY->ToLocation(axis2Placement, location);
					}
					
					if (hasEndPoint) {
						gp_Trsf trsf = location.Transformation();
						endPoint = gp_Pnt2d(0, 0).Transformed(trsf);
					}
				}

				if (hasEndPoint)
				{
					gp_Pnt2d lastPoint;
					Standard_Real lastParam = heightFunctionCurves.back()->LastParameter();
					heightFunctionCurves.back()->D0(lastParam, lastPoint);
					if (!lastPoint.IsEqual(endPoint, Precision::Confusion())) {
						Handle(Geom2d_Line) endSegment = new Geom2d_Line(lastPoint, gp_Vec2d(lastPoint, endPoint));
						Standard_Real length = lastPoint.Distance(endPoint);
						Handle(Geom2d_TrimmedCurve) trimmed = new Geom2d_TrimmedCurve(endSegment, 0, length);
						heightFunctionCurves.push_back(trimmed);
					}
				}

				TColGeom2d_SequenceOfBoundedCurve segmentsSequence = EXEC_NATIVE->GetSegmentsSequnce(heightFunctionCurves, ModelGeometryService->MinimumGap);
				Handle(Geom2d_BSplineCurve) heightFunction = EXEC_NATIVE->BuildCompositeCurve2d(segmentsSequence, ModelGeometryService->MinimumGap);

				if (heightFunction.IsNull())
					throw RaiseGeometryFactoryException("IfcGradientCurve segments could not be built", ifcGradientCurve);

				gp_Pnt2d pnt;
				heightFunction->D0(heightFunction->FirstParameter() , pnt);
				auto startDistAlong = pnt.X();
				horizontalProjection->D0(horizontalProjection->FirstParameter(), pnt);
				auto horizontalProjectionStart = pnt.X();

				if (horizontalProjectionStart != 0 || startDistAlong != 0) {
					EXEC_NATIVE->TranslateCurveStartPointToX(heightFunction, horizontalProjectionStart + startDistAlong);
				}

				Handle(Geom_GradientCurve) gradientCurve = new Geom_GradientCurve(horizontalProjection, heightFunction);

				GetCache()->Insert(ifcGradientCurve->EntityLabel, gradientCurve);

				return gradientCurve;
			}

			Handle(Geom_SegmentedReferenceCurve) CurveFactory::BuildCurve(Ifc4x3::GeometryResource::IfcSegmentedReferenceCurve^ ifcSegmentedReferenceCurve)
			{

				std::optional<Handle(Standard_Transient)> cached = GetCache()->Get(ifcSegmentedReferenceCurve->EntityLabel);
				if (cached.has_value()) {
					return Handle(Geom_SegmentedReferenceCurve)::DownCast(cached.value());
				}

				// Building the base curve
				Ifc4x3::GeometryResource::IfcGradientCurve^ baseGradientCurve = 
					dynamic_cast<Ifc4x3::GeometryResource::IfcGradientCurve^>(ifcSegmentedReferenceCurve->BaseCurve);

				if(baseGradientCurve == nullptr)
					throw RaiseGeometryFactoryException("IfcSegmentedReferenceCurve BaseCurve should be an IfcGradientCurve", ifcSegmentedReferenceCurve);

				Handle(Geom_GradientCurve) baseCurve = BuildCurve(baseGradientCurve);

				// Building the superelevation curve 
				for (int i = 0; i < ifcSegmentedReferenceCurve->NSegments; i++)
				{
					Ifc4x3::GeometryResource::IfcCurveSegment^ curveSegment = dynamic_cast<Ifc4x3::GeometryResource::IfcCurveSegment^>(ifcSegmentedReferenceCurve->Segments[i]);
					if (curveSegment == nullptr)
						throw RaiseGeometryFactoryException("IfcSegmentedReferenceCurve Segments should be of type IfcCurveSegment", ifcSegmentedReferenceCurve);
				}
				std::vector<std::pair<Handle(Geom2d_Curve), TopLoc_Location>> curveSegmentsWithLocations = ProcessSegments(ifcSegmentedReferenceCurve);

				bool hasEndPoint = false;
				TopLoc_Location endLocation;

				if (ifcSegmentedReferenceCurve->EndPoint != nullptr)
				{
					IfcAxis2PlacementLinear^ linearPlacement = dynamic_cast<IfcAxis2PlacementLinear^>(ifcSegmentedReferenceCurve->EndPoint);
					IIfcAxis2Placement3D^ axis2Placement = dynamic_cast<IIfcAxis2Placement3D^>(ifcSegmentedReferenceCurve->EndPoint);
					IIfcAxis2Placement2D^ axis2Placement2d = dynamic_cast<IIfcAxis2Placement2D^>(ifcSegmentedReferenceCurve->EndPoint);

					if (axis2Placement2d) {
						hasEndPoint = GEOMETRY_FACTORY->ToLocation(axis2Placement2d, endLocation);
					}
					else if (linearPlacement) {
						hasEndPoint = GEOMETRY_FACTORY->ToLocation(linearPlacement, endLocation);
					}
					else if (axis2Placement) {
						hasEndPoint = GEOMETRY_FACTORY->ToLocation(axis2Placement, endLocation);
					}
				}
				 

				TColGeom2d_SequenceOfBoundedCurve curves;
				for (size_t i = 0; i < curveSegmentsWithLocations.size(); i++)
				{
					curves.Append(Handle(Geom2d_BoundedCurve)::DownCast(curveSegmentsWithLocations[i].first));
				}

				Handle(Geom_SegmentedReferenceCurve) segmentedCurve = new Geom_SegmentedReferenceCurve
												(baseCurve, curveSegmentsWithLocations, endLocation, hasEndPoint);
			
				GetCache()->Insert(ifcSegmentedReferenceCurve->EntityLabel, segmentedCurve);

				return segmentedCurve;

			}

			Handle(Geom_BSplineCurve) CurveFactory::ToBSpline(Handle(Geom_Curve) curve, int nbPoints)
			{
				TColgp_Array1OfPnt points(1, nbPoints);
				double projectionParm1 = curve->FirstParameter();
				double projectionParm2 = curve->LastParameter();

				GeomAdaptor_Curve adaptorHorizontalProjection(curve, projectionParm1, projectionParm2);
				GCPnts_UniformAbscissa uniformAbscissa(adaptorHorizontalProjection, nbPoints);

				for (Standard_Integer i = 1; i <= nbPoints; ++i) {

					Standard_Real param = uniformAbscissa.Parameter(i);
					gp_Pnt pnt;
					curve->D0(param, pnt);
					points.SetValue(i, pnt);
				}

				GeomAPI_PointsToBSpline pointsToBSpline(points, 1, 8, GeomAbs_C1);
				return pointsToBSpline.Curve();
			}



			TopoDS_Edge CurveFactory::Convert2dToEdge(const Handle(Geom2d_Curve)& clothoid2d) {
				Standard_Integer numPoints = 1000;
				Standard_Real firstParam = clothoid2d->FirstParameter();
				Standard_Real lastParam = clothoid2d->LastParameter();

				TColgp_Array1OfPnt points3d(0, numPoints - 1);

				for (Standard_Integer i = 0; i < numPoints; ++i) {
					Standard_Real U = firstParam + ((lastParam - firstParam) / numPoints) * i;
					gp_Pnt2d P2d;
					clothoid2d->D0(U, P2d);
					gp_Pnt P3d(P2d.X(), P2d.Y(), 0.0); // Map to 3D by adding Z = 0
					points3d.SetValue(i, P3d);
				}

				Handle(Geom_BSplineCurve) bsplineCurve3d = GeomAPI_PointsToBSpline(points3d);
				TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(bsplineCurve3d);

				return edge;
			}

			TopoDS_Edge CurveFactory::ConvertToEdge(const Handle(Geom_Curve)& curve, Standard_Integer numPoints) {
				Standard_Real firstParam = curve->FirstParameter();
				Standard_Real lastParam = curve->LastParameter();

				TColgp_Array1OfPnt points(0, numPoints - 1);

				for (Standard_Integer i = 0; i < numPoints; ++i) {
					Standard_Real U = firstParam + ((lastParam - firstParam) / numPoints) * i;
					gp_Pnt P;
					curve->D0(U, P);
					points.SetValue(i, P);
				}

				Handle(Geom_BSplineCurve) bsplineCurve3d = GeomAPI_PointsToBSpline(points);
				TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(bsplineCurve3d);

				return edge;
			}


			TopoDS_Shape CurveFactory::ConvertToShape(const Handle(Geom_Curve)& curve, Standard_Integer numPoints) {
				Standard_Real firstParam = curve->FirstParameter();
				Standard_Real lastParam = curve->LastParameter();

				TColgp_Array1OfPnt points(0, numPoints - 1);
				for (Standard_Integer i = 0; i < numPoints; ++i) {
					Standard_Real U = firstParam + ((lastParam - firstParam) / numPoints) * i;
					gp_Pnt P;
					curve->D0(U, P);
					points.SetValue(i, P);
				}
				TopoDS_Compound compound;
				BRep_Builder builder;
				builder.MakeCompound(compound);

				for (Standard_Integer i = points.Lower(); i < points.Upper(); ++i) {
					gp_Pnt p1 = points.Value(i);
					gp_Pnt p2 = points.Value(i + 1);
					TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
					builder.Add(compound, edge);
				}

				return compound;
			}

			TopoDS_Shape CurveFactory::Convert2dCurvesToShape(const TColGeom2d_SequenceOfBoundedCurve& curves2d) {
				BRep_Builder builder;
				TopoDS_Compound compound;
				builder.MakeCompound(compound);

				for (Standard_Integer i = 1; i <= curves2d.Length(); ++i) {
					Handle(Geom2d_BoundedCurve) curve2d = curves2d.Value(i);

					if (curve2d.IsNull()) {
						continue;
					}

					TopoDS_Edge edge = Convert2dToEdge(curve2d);;
					builder.Add(compound, edge);
				}

				return compound;
			}

			TopoDS_Shape CurveFactory::ConvertCurvesToShape(const TColGeom_SequenceOfBoundedCurve& curves2d) {
				BRep_Builder builder;
				TopoDS_Compound compound;
				builder.MakeCompound(compound);

				for (Standard_Integer i = 1; i <= curves2d.Length(); ++i) {
					Handle(Geom_BoundedCurve) curve2d = curves2d.Value(i);

					if (curve2d.IsNull()) {
						continue;
					}
					int n = (int)((curve2d.get()->LastParameter() - curve2d.get()->FirstParameter()) / (100 * _modelService->OneMillimeter));
					TopoDS_Edge edge = ConvertToEdge(curve2d, n);;
					builder.Add(compound, edge);
				}

				return compound;
			}

			std::vector<Handle(Geom2d_Curve)> CurveFactory::ProcessSegments(Ifc4x3::GeometryResource::IfcGradientCurve^ ifcGradientCurve)
			{
				std::vector<Handle(Geom2d_Curve)> verticalProfile;
				for (int i = 0; i < ifcGradientCurve->NSegments; i++)
				{
					Ifc4x3::GeometryResource::IfcCurveSegment^ segment = dynamic_cast<Ifc4x3::GeometryResource::IfcCurveSegment^>(ifcGradientCurve->Segments[i]);
					// Gradient height function segments are 2d curves
					IIfcAxis2Placement2D^ axis2 = dynamic_cast<IIfcAxis2Placement2D^>(segment->Placement);
					if (axis2 == nullptr)
						throw RaiseGeometryFactoryException("IfcGradientCurve Segments placement should be of IfcAxis2Placement2D type", segment);
					Handle(Geom2d_Curve) curveSegment = BuildCurveSegment2d(segment);
					if (!curveSegment)
						continue;
					TransformCurveWithLocation(curveSegment, axis2);
					verticalProfile.push_back(curveSegment);
				}
				return verticalProfile;
			}

			std::vector<std::pair<Handle(Geom2d_Curve), TopLoc_Location>> CurveFactory::ProcessSegments(Ifc4x3::GeometryResource::IfcSegmentedReferenceCurve^ ifcSegmentedReferenceCurve)
			{
				std::vector<std::pair<Handle(Geom2d_Curve), TopLoc_Location>>  superElevationProfile;
				for (int i = 0; i < ifcSegmentedReferenceCurve->NSegments; i++)
				{
					Ifc4x3::GeometryResource::IfcCurveSegment^ segment = dynamic_cast<Ifc4x3::GeometryResource::IfcCurveSegment^>(ifcSegmentedReferenceCurve->Segments[i]);
					// Segmented curve superelevation segments
					IIfcAxis2Placement3D^ axis2 = dynamic_cast<IIfcAxis2Placement3D^>(segment->Placement);
					if (axis2 == nullptr)
						throw RaiseGeometryFactoryException("IfcSegmentedReferenceCurve Segments placement should be of IfcAxis2Placement3D type", segment);

					TopLoc_Location location;
					GEOMETRY_FACTORY->ToLocation(axis2, location);
					Handle(Geom2d_Curve) curveSegment = BuildCurveSegment2d(segment);

					if (!curveSegment)
						continue;

					curveSegment = TransformCurveWithLocation(curveSegment, axis2);
					superElevationProfile.emplace_back(curveSegment, location);

				}
				return superElevationProfile;
			}

			Handle(Geom2d_Curve) CurveFactory::TransformCurveWithLocation(const Handle(Geom2d_Curve)& curve, IIfcAxis2Placement2D^ placement)
			{
				gp_Pnt2d translation;
				if (!GEOMETRY_FACTORY->BuildPoint2d(placement->Location, translation))
					throw RaiseGeometryFactoryException("IIfcAxis2Placement2D Location must be 2D", placement);
				gp_XY xDir(1, 0);
				if (placement->RefDirection != nullptr)
					xDir = gp_XY(placement->RefDirection->DirectionRatios[0], placement->RefDirection->DirectionRatios[1]);
				gp_Trsf2d transform2d;
				gp_Ax2d axis2d(translation, xDir);
				gp_Ax2d globalAxis(gp::Origin2d(), gp_Dir2d(1, 0));
				transform2d.SetTransformation(axis2d, globalAxis);
				curve->Transform(transform2d);
				return curve;
			}

			Handle(Geom2d_Curve) CurveFactory::TransformCurveWithLocation(const Handle(Geom2d_Curve)& curve, IIfcAxis2Placement3D^ placement)
			{
				gp_Pnt translation = GEOMETRY_FACTORY->BuildPoint3d(placement->Location);
				gp_XYZ xDir(1, 0, 0);
				gp_XYZ zDir(0, 0, 1);
				if (placement->RefDirection != nullptr)
					xDir = gp_XYZ(placement->RefDirection->DirectionRatios[0], placement->RefDirection->DirectionRatios[1], placement->RefDirection->DirectionRatios[2]);
				if (placement->Axis != nullptr)
					zDir = gp_XYZ(placement->Axis->DirectionRatios[0], placement->Axis->DirectionRatios[1], placement->Axis->DirectionRatios[2]);
				gp_Trsf transform;
				gp_Ax3 axis(translation, zDir, xDir);
				transform.SetTransformation(axis, gp_Ax3(gp_Pnt(), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)));
				curve->Transform(transform);
				return curve;
			}

			Handle(Geom_Curve) CurveFactory::TrimCurveByWires(const Handle(Geom_Curve)& curve, const TopoDS_Wire& wire1, const TopoDS_Wire& wire2)
			{
				return EXEC_NATIVE->TrimCurveByWires(curve, wire1, wire2);
			}

			Handle(Geom_Curve) CurveFactory::TrimCurveByFaces(const Handle(Geom_Curve)& curve, const TopoDS_Face& face1, const TopoDS_Face& face2)
			{
				return EXEC_NATIVE->TrimCurveByFaces(curve, face1, face2);
			}

			Handle(Geom_Curve) CurveFactory::TrimCurveByAtDistances(const Handle(Geom_Curve)& curve, Standard_Real distance1, Standard_Real distance2)
			{
				if (distance1 < 0 || distance2 < 0) {
					Standard_Failure::Raise("Distances must be non-negative.");
					return curve; // Not reached, but added for completeness
				}

				Standard_Real firstParam = curve->FirstParameter();
				Standard_Real lastParam = curve->LastParameter();

				auto convertibleToBSpline = Handle(Geom_ConvertibleToBSpline)::DownCast(curve);

				if (convertibleToBSpline) {
					Handle(Geom_TrimmedCurve) trimmedCurve = new Geom_TrimmedCurve(curve, firstParam + distance1, firstParam + distance2);
					return trimmedCurve;
				}

				GeomAdaptor_Curve adaptor(curve, firstParam, lastParam);
				Standard_Real totalLength = GCPnts_AbscissaPoint::Length(adaptor, firstParam, lastParam);

				if (curve->IsClosed()) {
					distance1 = fmod(distance1, totalLength);
					distance2 = fmod(distance2, totalLength);
					if (distance1 < 0) distance1 += totalLength;
					if (distance2 < 0) distance2 += totalLength;
				}

				if (distance1 > distance2) {
					std::swap(distance1, distance2);
				}

				if (distance1 - totalLength > Precision::Confusion() || distance2 - totalLength > Precision::Confusion()) {
					Standard_Failure::Raise("Distances exceed the total length of the curve.");
					return curve;
				}

				GCPnts_AbscissaPoint abscissaPoint1(adaptor, distance1, firstParam, lastParam);
				if (!abscissaPoint1.IsDone()) {
					Standard_Failure::Raise("Failed to compute parameter for the first distance.");
					return curve;
				}
				Standard_Real param1 = abscissaPoint1.Parameter();

				GCPnts_AbscissaPoint abscissaPoint2(adaptor, distance2, firstParam, lastParam);
				if (!abscissaPoint2.IsDone()) {
					Standard_Failure::Raise("Failed to compute parameter for the second distance.");
					return curve; 
				}
				Standard_Real param2 = abscissaPoint2.Parameter();

				Handle(Geom_TrimmedCurve) trimmedCurve = new Geom_TrimmedCurve(curve, param1, param2);

				return trimmedCurve;
			}


			Handle(Geom_BSplineCurve) CurveFactory::BuildCurve(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots)
			{
				if (dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(ifcBSplineCurveWithKnots))
					return BuildCurve(static_cast<IIfcRationalBSplineCurveWithKnots^>(ifcBSplineCurveWithKnots));
				//all control points shall have the same dimensionality of 3
				int numPoles = ifcBSplineCurveWithKnots->ControlPointsList->Count;
				int numKnots = ifcBSplineCurveWithKnots->Knots->Count;
				int numKnotMultiplicities = ifcBSplineCurveWithKnots->KnotMultiplicities->Count;

				if (numKnots != numKnotMultiplicities)
					throw RaiseGeometryFactoryException("Rule CorrespondingKnotLists: The number of elements in the knot multiplicities list shall be equal to the number of elements in the knots list", ifcBSplineCurveWithKnots);

				TColgp_Array1OfPnt poles(1, numPoles);
				TColStd_Array1OfReal knots(1, numKnots);
				TColStd_Array1OfInteger knotMultiplicities(1, numKnotMultiplicities);

				int i = 1;
				for each (IIfcCartesianPoint ^ cp in ifcBSplineCurveWithKnots->ControlPointsList)
				{
					if ((int)cp->Dim != 3)
						throw RaiseGeometryFactoryException("Rule SameDim: All control points shall have the same dimensionality.", ifcBSplineCurveWithKnots);
					poles.SetValue(i, GEOMETRY_FACTORY->BuildPoint3d(cp));
					i++;
				}

				i = 1;
				for each (double knot in ifcBSplineCurveWithKnots->Knots)
				{
					knots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (int multiplicity in ifcBSplineCurveWithKnots->KnotMultiplicities)
				{
					knotMultiplicities.SetValue(i, multiplicity);
					i++;
				}
				Handle(Geom_BSplineCurve) bSpline = OccHandle().BuildBSplineCurve3d(poles, knots, knotMultiplicities, (int)ifcBSplineCurveWithKnots->Degree);
				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("Failed to build IfcBSplineCurveWithKnots", ifcBSplineCurveWithKnots);
				return bSpline;
			}

			Handle(Geom_Circle) CurveFactory::BuildCurve(IIfcCircle^ ifcCircle)
			{
				if (ifcCircle->Radius <= 0)
					throw RaiseGeometryFactoryException("Circle radius cannot be <= 0.", ifcCircle);
				gp_Ax2 pos = GEOMETRY_FACTORY->BuildAxis2Placement(ifcCircle->Position); //throws exception
				Handle(Geom_Circle) circle = OccHandle().BuildCircle3d(pos, ifcCircle->Radius);
				if (circle.IsNull())
					throw RaiseGeometryFactoryException("Failed to build valid IIfcCircle", ifcCircle);
				return circle;
			}

			Handle(Geom_BSplineCurve) CurveFactory::BuildCurve(IIfcCompositeCurve^ ifcCompositeCurve)
			{
				XCurveType curveType;
				TColGeom_SequenceOfBoundedCurve segments;
				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{
					IIfcReparametrisedCompositeCurveSegment^ reparameterisedSegment = dynamic_cast<IIfcReparametrisedCompositeCurveSegment^>(segment);
					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (!IsBoundedCurve(segment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
					Handle(Geom_Curve) hSegment = BuildCurve(segment->ParentCurve, curveType);
					if (hSegment.IsNull())
						throw RaiseGeometryFactoryException("Composite curve segment is incorrectly defined", segment);
					Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(hSegment);
					if (boundedCurve.IsNull())
						throw RaiseGeometryFactoryException("Compound curve segments must be bounded curves", segment);
					if (!segment->SameSense)
						boundedCurve->Reverse();
					segments.Append(boundedCurve);
				}
				Handle(Geom_BSplineCurve) bSpline = OccHandle().BuildCompositeCurve3d(segments, ModelGeometryService->MinimumGap); //use minimum gap for tolerance to avoid issues with curves and line tolerance errors
				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("Composite curve could not be built", ifcCompositeCurve);
				return bSpline;
			}

			Handle(Geom_BSplineCurve) CurveFactory::BuildCurve(Ifc4x3::GeometryResource::IfcCompositeCurve^ ifcCompositeCurve)
			{
				XCurveType curveType;
				TColGeom_SequenceOfBoundedCurve segments;
				for each (Ifc4x3::GeometryResource::IfcSegment^ segment in ifcCompositeCurve->Segments)
				{
					Ifc4x3::GeometryResource::IfcReparametrisedCompositeCurveSegment^ reparameterisedSegment
						= dynamic_cast<Ifc4x3::GeometryResource::IfcReparametrisedCompositeCurveSegment^>(segment);

					Ifc4x3::GeometryResource::IfcCompositeCurveSegment^ compositeSegment
						= dynamic_cast<Ifc4x3::GeometryResource::IfcCompositeCurveSegment^>(segment);

					Ifc4x3::GeometryResource::IfcCurveSegment^ curveSegment
						= dynamic_cast<Ifc4x3::GeometryResource::IfcCurveSegment^>(segment);


					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (compositeSegment != nullptr && !IsBoundedCurve(compositeSegment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");

					if (compositeSegment != nullptr)
					{
						Handle(Geom_Curve) hSegment = BuildCurve(compositeSegment->ParentCurve, curveType);
						if (hSegment.IsNull())
							throw RaiseGeometryFactoryException("Composite curve segment is incorrectly defined", segment);
						Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(hSegment);
						if (boundedCurve.IsNull())
							throw RaiseGeometryFactoryException("Compound curve segments must be bounded curves", segment);
						if (!compositeSegment->SameSense)
							boundedCurve->Reverse();
						segments.Append(boundedCurve);
					}
					else if (curveSegment != nullptr)
					{
						//TODO: build segment 3d
						//segments.Append(seg);

					}
				}

				Handle(Geom_BSplineCurve) bSpline = OccHandle().BuildCompositeCurve3d(segments, ModelGeometryService->MinimumGap); //use minimum gap for tolerance to avoid issues with curves and line tolerance errors
				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("Composite curve could not be built", ifcCompositeCurve);
				return bSpline;
			}

			Handle(Geom_BSplineCurve) CurveFactory::BuildCurve(IIfcCompositeCurveOnSurface^ ifcCompositeCurveOnSurface)
			{
				throw gcnew NotImplementedException("BuildCurve(IfcCompositeCurveOnSurface)");
			}

			Handle(Geom_Ellipse) CurveFactory::BuildCurve(IIfcEllipse^ ifcEllipse)
			{
				gp_Ax2 pos = GEOMETRY_FACTORY->BuildAxis2Placement(ifcEllipse->Position);
				Handle(Geom_Ellipse) elipse = OccHandle().BuildEllipse3d(pos, ifcEllipse->SemiAxis1, ifcEllipse->SemiAxis2);
				if (elipse.IsNull())
					throw RaiseGeometryFactoryException("Failed to build IfcEllipse", ifcEllipse);
				return elipse;
			}

			Handle(Geom_BSplineCurve) CurveFactory::BuildCurve(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve)
			{
				TColGeom_SequenceOfBoundedCurve segments;
				BuildIndexPolyCurveSegments3d(ifcIndexedPolyCurve, segments); //this may throw exceptions
				Handle(Geom_BSplineCurve) bspline = OccHandle().BuildIndexedPolyCurve3d(segments, ModelGeometryService->MinimumGap);
				if (bspline.IsNull())
					throw RaiseGeometryFactoryException("IIfcIndexedPolyCurve could not be built", ifcIndexedPolyCurve);
				return bspline;
			}

			Handle(Geom_LineWithMagnitude) CurveFactory::BuildCurve(IIfcLine^ ifcLine)
			{
				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint3d(ifcLine->Pnt);
				gp_Vec direction;
				if (!GEOMETRY_FACTORY->BuildDirection3d(ifcLine->Dir->Orientation, direction))
					throw RaiseGeometryFactoryException("Line orientation could not be built", ifcLine->Dir->Orientation);
				return Ptr()->BuildLine3d(origin, direction, ifcLine->Dir->Magnitude);

			}

			Handle(Geom_OffsetCurve) CurveFactory::BuildCurve(IIfcOffsetCurve2D^ ifcOffsetCurve2D)
			{
				Standard_Real Offset = ifcOffsetCurve2D->Distance;
				XCurveType curveType;
				Handle(Geom_Curve) cBasis = BuildCurve(ifcOffsetCurve2D->BasisCurve, curveType);
				Handle(Geom_OffsetCurve) offsetCurve = new Geom_OffsetCurve(cBasis, Offset, gp::DZ()); //it is 2d so will be in the xy plane
				if (offsetCurve.IsNull())
					throw RaiseGeometryFactoryException("Cannot build offset curve, see logs", ifcOffsetCurve2D);
				return offsetCurve;
			}

			Handle(Geom_OffsetCurve) CurveFactory::BuildCurve(IIfcOffsetCurve3D^ ifcOffsetCurve3D)
			{
				XCurveType curveType;
				Handle(Geom_Curve) basisCurve = BuildCurve(ifcOffsetCurve3D->BasisCurve, curveType); //throws exception
				gp_Vec refDir;
				if (!GEOMETRY_FACTORY->BuildDirection3d(ifcOffsetCurve3D->RefDirection, refDir))
					throw RaiseGeometryFactoryException("Cannot build offset curve reference direction", ifcOffsetCurve3D->RefDirection);
				Handle(Geom_OffsetCurve) offsetCurve = OccHandle().BuildOffsetCurve3d(basisCurve, refDir, ifcOffsetCurve3D->Distance);
				if (offsetCurve.IsNull())
					throw RaiseGeometryFactoryException("Cannot build offset curve, see logs", ifcOffsetCurve3D);
				return offsetCurve;
			}

			Handle(Geom_Curve) CurveFactory::BuildCurve(IIfcPcurve^ ifcPcurve)
			{
				throw gcnew NotImplementedException("BuildCurve(IfcPcurve)");
			}

			Handle(Geom_Curve) CurveFactory::BuildCurve(IIfcPolyline^ ifcPolyline)
			{
				//validate
				int pointCount = ifcPolyline->Points->Count;
				if (pointCount < 2)
					throw RaiseGeometryFactoryException("IfcPolyline has less than 2 points. It cannot be built", ifcPolyline);
				if (pointCount == 2) //just build a line
				{
					gp_Pnt start = GEOMETRY_FACTORY->BuildPoint3d(ifcPolyline->Points[0]);
					gp_Pnt end = GEOMETRY_FACTORY->BuildPoint3d(ifcPolyline->Points[1]);
					if (start.IsEqual(end, ModelGeometryService->Precision))
						LogInformation(ifcPolyline, "IfcPolyline has only 2 identical points. It has been ignored");
					Handle(Geom_TrimmedCurve) lineSeg = OccHandle().BuildTrimmedLine3d(start, end);
					if (lineSeg.IsNull())
						throw RaiseGeometryFactoryException("Invalid IfcPolyline definition", ifcPolyline);
					return lineSeg;
				}
				else
				{
					TColgp_Array1OfPnt points(1, ifcPolyline->Points->Count);
					GEOMETRY_FACTORY->GetPolylinePoints3d(ifcPolyline, points);
					Handle(Geom_BSplineCurve) polyline = Ptr()->BuildPolyline3d(points, ModelGeometryService->Precision);
					if (polyline.IsNull())
						throw RaiseGeometryFactoryException("Failed to build IfcPolyline", ifcPolyline);
					return polyline;
				}

			}

			Handle(Geom_BSplineCurve) CurveFactory::BuildCurve(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots)
			{
				TColgp_Array1OfPnt poles(1, ifcRationalBSplineCurveWithKnots->ControlPointsList->Count);
				TColStd_Array1OfReal knots(1, ifcRationalBSplineCurveWithKnots->Knots->Count);
				TColStd_Array1OfInteger knotMultiplicities(1, ifcRationalBSplineCurveWithKnots->Knots->Count);
				TColStd_Array1OfReal weights(1, ifcRationalBSplineCurveWithKnots->ControlPointsList->Count);
				int i = 1;
				for each (IIfcCartesianPoint ^ cp in ifcRationalBSplineCurveWithKnots->ControlPointsList)
				{
					poles.SetValue(i, GEOMETRY_FACTORY->BuildPoint3d(cp));
					i++;
				}

				i = 1;
				for each (double knot in ifcRationalBSplineCurveWithKnots->Knots)
				{
					knots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (int multiplicity in ifcRationalBSplineCurveWithKnots->KnotMultiplicities)
				{
					knotMultiplicities.SetValue(i, multiplicity);
					i++;
				}
				i = 1;
				for each (double weight in ifcRationalBSplineCurveWithKnots->Weights)
				{
					weights.SetValue(i, weight);
					i++;
				}

				Handle(Geom_BSplineCurve) bspline = OccHandle().BuildRationalBSplineCurve3d(poles, weights, knots, knotMultiplicities, (int)ifcRationalBSplineCurveWithKnots->Degree);
				if (bspline.IsNull())
					throw RaiseGeometryFactoryException("IIfcRationalBSplineCurveWithKnots could not be built.", ifcRationalBSplineCurveWithKnots);
				return bspline;
			}

			Handle(Geom_Curve) CurveFactory::BuildCurve(IIfcSurfaceCurve^ ifcPolyline)
			{
				throw gcnew NotImplementedException("BuildCurve(IfcSurfaceCurve)");
			}

			Handle(Geom_TrimmedCurve) CurveFactory::BuildCurve(IIfcTrimmedCurve^ ifcTrimmedCurve)
			{
				//Validation
				if (dynamic_cast<IIfcBoundedCurve^>(ifcTrimmedCurve->BasisCurve))
					LogDebug(ifcTrimmedCurve, "Ifc Formal Proposition: NoTrimOfBoundedCurves. Already bounded curves shall not be trimmed is violated, but processing has continued");
				XCurveType curveType;
				Handle(Geom_Curve) basisCurve = BuildCurve(ifcTrimmedCurve->BasisCurve, curveType);
				if (!basisCurve.IsNull())
				{
					bool isConic = (dynamic_cast<IIfcConic^>(ifcTrimmedCurve->BasisCurve) != nullptr);
					bool isLine = (dynamic_cast<IIfcLine^>(ifcTrimmedCurve->BasisCurve) != nullptr);
					bool isEllipse = (dynamic_cast<IIfcEllipse^>(ifcTrimmedCurve->BasisCurve) != nullptr);

					bool sense = ifcTrimmedCurve->SenseAgreement;
					//get the parametric values
					Xbim::Ifc4::Interfaces::IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

					bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == Xbim::Ifc4::Interfaces::IfcTrimmingPreference::CARTESIAN);

					double u1 = double::NegativeInfinity, u2 = double::PositiveInfinity;
					IIfcCartesianPoint^ cp1 = nullptr;
					IIfcCartesianPoint^ cp2 = nullptr;

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
						gp_Pnt p1 = GEOMETRY_FACTORY->BuildPoint3d(cp1);
						gp_Pnt p2 = GEOMETRY_FACTORY->BuildPoint3d(cp2);
						if (!GeomLib_Tool::Parameter(basisCurve, p1, ModelGeometryService->MinimumGap, u1))
							throw RaiseGeometryFactoryException("Trim Point1 is not on the basis curve", ifcTrimmedCurve);
						if (!GeomLib_Tool::Parameter(basisCurve, p2, ModelGeometryService->MinimumGap, u2))
							throw RaiseGeometryFactoryException("Trim Point2 is not on the basis curve", ifcTrimmedCurve);

					}
					else if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //non-compliant
						throw RaiseGeometryFactoryException("Ifc Formal Proposition: TrimValuesConsistent. Either a single value is specified for Trim, or the two trimming values are of different type (point and parameter)", ifcTrimmedCurve);
					else //we prefer to use parameters but need to adjust
					{
						if (isConic)
						{
							u1 *= ModelGeometryService->RadianFactor; //correct to radians
							u2 *= ModelGeometryService->RadianFactor; //correct to radians

						}
					}

					if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //sanity check in case the logic has missed a situtation
						throw RaiseGeometryFactoryException("Error converting Ifc Trim Points", ifcTrimmedCurve);

					if (Math::Abs(u1 - u2) < Precision::Confusion()) //if the parameters are the same trimming will fail if not a conic curve
					{
						if (isConic)
							return Ptr()->BuildTrimmedCurve3d(basisCurve, 0, Math::PI * 2, true); //return a full circle
						else
						{
							LogInformation(ifcTrimmedCurve->BasisCurve, "Parametric Trim Points are equal and will result in an empty curve");
							return Handle(Geom_TrimmedCurve)();
						}
					}
					else
						return Ptr()->BuildTrimmedCurve3d(basisCurve, u1, u2, sense);
				}
				else
					throw RaiseGeometryFactoryException("Failed to build Trimmed Basis Curve", ifcTrimmedCurve->BasisCurve);

			}

#pragma endregion


#pragma region Spirals


			Handle(Geom2d_Polynomial) CurveFactory::BuildBoundedPolynomialCurve
				(Ifc4x3::GeometryResource::IfcPolynomialCurve^ curve, Standard_Real startParam, Standard_Real endParam)
			{
				if (!curve->CoefficientsX || !curve->CoefficientsY) {
					throw RaiseGeometryFactoryException("CoefficientsX and CoefficientsY must be defined", curve);
				}
				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(curve->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Polynomial curves must have a Axis2Placement2D placement", curve);

				GEOMETRY_FACTORY->BuildAxis2Placement2d(placement, position);

				std::vector<Standard_Real> coeffX;
				std::vector<Standard_Real> coeffY;

				for (int i = 0; i < curve->CoefficientsX->Count; i++)
				{
					coeffX.push_back(curve->CoefficientsX[i]);
				}

				for (int i = 0; i < curve->CoefficientsY->Count; i++)
				{
					coeffY.push_back(curve->CoefficientsY[i]);
				}

				Handle(Geom2d_Polynomial) polyCurve = new Geom2d_Polynomial(position, coeffX, coeffY, startParam, endParam);

				return polyCurve;
			}


			Handle(Geom2d_Spiral) CurveFactory::BuildBoundedSpiral
				(Ifc4x3::GeometryResource::IfcSpiral^ ifcSpiral, Standard_Real startParam, Standard_Real endParam, XCurveType& curveType)
			{

				if (!Enum::TryParse<XCurveType>(static_cast<IPersistEntity^>(ifcSpiral)->ExpressType->ExpressName, curveType))
					throw RaiseGeometryFactoryException("Unsupported curve type", ifcSpiral);


				switch (curveType)
				{
					case XCurveType::IfcClothoid: {
						Ifc4x3::GeometryResource::IfcClothoid^ clothoid = dynamic_cast<Ifc4x3::GeometryResource::IfcClothoid^>(ifcSpiral);
						return BuildClothoid(clothoid, startParam, endParam);
					}
					case XCurveType::IfcSecondOrderPolynomialSpiral: {
						Ifc4x3::GeometryResource::IfcSecondOrderPolynomialSpiral^ polynomialSpiral
								= dynamic_cast<Ifc4x3::GeometryResource::IfcSecondOrderPolynomialSpiral^>(ifcSpiral);
						return BuildPolynomialSpiral(polynomialSpiral, startParam, endParam);
					}
					case XCurveType::IfcThirdOrderPolynomialSpiral: {
						Ifc4x3::GeometryResource::IfcThirdOrderPolynomialSpiral^ polynomialSpiral
							= dynamic_cast<Ifc4x3::GeometryResource::IfcThirdOrderPolynomialSpiral^>(ifcSpiral);
						return BuildPolynomialSpiral(polynomialSpiral, startParam, endParam);
					}
					case XCurveType::IfcSeventhOrderPolynomialSpiral: {
						Ifc4x3::GeometryResource::IfcSeventhOrderPolynomialSpiral^ polynomialSpiral
							= dynamic_cast<Ifc4x3::GeometryResource::IfcSeventhOrderPolynomialSpiral^>(ifcSpiral);
						return BuildPolynomialSpiral(polynomialSpiral, startParam, endParam);
					}
					case XCurveType::IfcSineSpiral: {
						Ifc4x3::GeometryResource::IfcSineSpiral^ sineSpiral
							= dynamic_cast<Ifc4x3::GeometryResource::IfcSineSpiral^>(ifcSpiral);
						return BuildSineSpiral(sineSpiral, startParam, endParam);
					}
					case XCurveType::IfcCosineSpiral: {
						Ifc4x3::GeometryResource::IfcCosineSpiral^ cosineSpiral
							= dynamic_cast<Ifc4x3::GeometryResource::IfcCosineSpiral^>(ifcSpiral);
						return BuildCosineSpiral(cosineSpiral, startParam, endParam);
					}
					default:
						throw RaiseGeometryFactoryException("Unknown spiral curve type", ifcSpiral);
				}
			}

			Handle(Geom2d_SineSpiral) CurveFactory::BuildSineSpiral(Ifc4x3::GeometryResource::IfcSineSpiral^ sineSpiral, Standard_Real startParam, Standard_Real endParam)
			{

				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(sineSpiral->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Sine Spirals must have a Axis2Placement2D placement", sineSpiral);

				Standard_Real constantTerm = sineSpiral->ConstantTerm.HasValue
					? (Standard_Real)sineSpiral->ConstantTerm.Value.Value
					: 0;

				Standard_Real linearTerm = sineSpiral->LinearTerm.HasValue
					? (Standard_Real)sineSpiral->LinearTerm.Value.Value
					: 0;

				Standard_Real sineTerm = (Standard_Real)sineSpiral->SineTerm.Value;

				Handle(Geom2d_SineSpiral) spiral = new Geom2d_SineSpiral(position, sineTerm, linearTerm, constantTerm, startParam, endParam);

				return spiral;
			}

			Handle(Geom2d_CosineSpiral) CurveFactory::BuildCosineSpiral(Ifc4x3::GeometryResource::IfcCosineSpiral^ cosineSpiral, Standard_Real startParam, Standard_Real endParam)
			{

				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(cosineSpiral->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Coine Spirals must have a Axis2Placement2D placement", cosineSpiral);

				Standard_Real constantTerm = cosineSpiral->ConstantTerm.HasValue
					? (Standard_Real)cosineSpiral->ConstantTerm.Value.Value
					: 0;
				 

				Standard_Real cosineTerm = (Standard_Real)cosineSpiral->CosineTerm.Value;

				Handle(Geom2d_CosineSpiral) spiral = new Geom2d_CosineSpiral(position, cosineTerm, constantTerm, startParam, endParam);

				return spiral;
			}


			Handle(Geom2d_PolynomialSpiral) CurveFactory::BuildPolynomialSpiral(Ifc4x3::GeometryResource::IfcSecondOrderPolynomialSpiral^ polynomialSpiral, Standard_Real startParam, Standard_Real endParam)
			{
				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(polynomialSpiral->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Polynomial Spirals must have a Axis2Placement2D placement", polynomialSpiral);

				std::vector<std::optional<Standard_Real>> coefficients{
					// A0
					polynomialSpiral->ConstantTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->ConstantTerm.Value.Value)
					: std::nullopt,
					// A1
					polynomialSpiral->LinearTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->LinearTerm.Value.Value)
					: std::nullopt,
					// A2
					std::make_optional<Standard_Real>((double)(polynomialSpiral->QuadraticTerm.Value)),
				};


				Handle(Geom2d_PolynomialSpiral) spiral = new Geom2d_PolynomialSpiral(position, coefficients, startParam, endParam);

				return spiral;

			}

			Handle(Geom2d_PolynomialSpiral) CurveFactory::BuildPolynomialSpiral(Ifc4x3::GeometryResource::IfcThirdOrderPolynomialSpiral^ polynomialSpiral, Standard_Real startParam, Standard_Real endParam)
			{
				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(polynomialSpiral->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Polynomial Spirals must have a Axis2Placement2D placement", polynomialSpiral);

				std::vector<std::optional<Standard_Real>> coefficients{
					// A0
					polynomialSpiral->ConstantTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->ConstantTerm.Value.Value)
					: std::nullopt,
					// A1
					polynomialSpiral->LinearTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->LinearTerm.Value.Value)
					: std::nullopt,
					// A2
					polynomialSpiral->QuadraticTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->QuadraticTerm.Value.Value)
					: std::nullopt,
					// A3
					std::make_optional<Standard_Real>((double)(polynomialSpiral->CubicTerm.Value)),
				};


				Handle(Geom2d_PolynomialSpiral) spiral = new Geom2d_PolynomialSpiral(position, coefficients, startParam, endParam);

				return spiral;
			}

			Handle(Geom2d_PolynomialSpiral) CurveFactory::BuildPolynomialSpiral(Ifc4x3::GeometryResource::IfcSeventhOrderPolynomialSpiral^ polynomialSpiral, Standard_Real startParam, Standard_Real endParam)
			{
				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(polynomialSpiral->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Polynomial Spirals must have a Axis2Placement2D placement", polynomialSpiral);

				std::vector<std::optional<Standard_Real>> coefficients{
					// A0
					polynomialSpiral->ConstantTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->ConstantTerm.Value.Value)
					: std::nullopt,
					// A1
					polynomialSpiral->LinearTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->LinearTerm.Value.Value)
					: std::nullopt,
					// A2
					polynomialSpiral->QuadraticTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->QuadraticTerm.Value.Value)
					: std::nullopt,
					// A3
					polynomialSpiral->CubicTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->CubicTerm.Value.Value)
					: std::nullopt,
					// A4
					polynomialSpiral->QuarticTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->QuarticTerm.Value.Value)
					: std::nullopt,
					// A5
					polynomialSpiral->QuinticTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->QuinticTerm.Value.Value)
					: std::nullopt,
					// A6
					polynomialSpiral->SexticTerm.HasValue
					? std::make_optional<Standard_Real>((double)polynomialSpiral->SexticTerm.Value.Value)
					: std::nullopt,
					// A7
					std::make_optional<Standard_Real>((double)(polynomialSpiral->SepticTerm.Value)),
				};


				Handle(Geom2d_PolynomialSpiral) spiral = new Geom2d_PolynomialSpiral(position, coefficients, startParam, endParam);

				return spiral;
			}

			Handle(Geom2d_Clothoid) CurveFactory::BuildClothoid(Ifc4x3::GeometryResource::IfcClothoid^ ifcClothoid, Standard_Real startParam, Standard_Real endParam)
			{
				gp_Ax22d position;
				IIfcAxis2Placement2D^ placement = dynamic_cast<IIfcAxis2Placement2D^>(ifcClothoid->Position);

				if (placement == nullptr)
					throw RaiseGeometryFactoryException("Clothoids must have a Axis2Placement2D placement", ifcClothoid);

				GEOMETRY_FACTORY->BuildAxis2Placement2d(placement, position);
				Standard_Real clothoidConstant = ifcClothoid->ClothoidConstant;

				Handle(Geom2d_Clothoid) clothoid = new Geom2d_Clothoid(position, clothoidConstant, startParam, endParam);

				return clothoid;
			}


#pragma endregion


#pragma region Build 2D Curves

			Handle(Geom2d_Curve) CurveFactory::BuildCurve2d(IIfcCurve^ curve)
			{
				XCurveType curveType;
				return BuildCurve2d(curve, curveType);
			}

			Handle(Geom2d_Curve) CurveFactory::BuildCurve2d(IIfcCurve^ curve, XCurveType% curveType)
			{
				if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
					throw RaiseGeometryFactoryException("Unsupported curve type", curve);

				switch (curveType)
				{
					case XCurveType::IfcBSplineCurveWithKnots:
						return BuildCurve2d(static_cast<IIfcBSplineCurveWithKnots^>(curve));
					case XCurveType::IfcCircle:
						return BuildCurve2d(static_cast<IIfcCircle^>(curve));
					case XCurveType::IfcCompositeCurve:
					{
						Ifc4x3::GeometryResource::IfcCompositeCurve^ ifc4x3Curve = dynamic_cast<Ifc4x3::GeometryResource::IfcCompositeCurve^>(curve);
						if (ifc4x3Curve != nullptr)
							return BuildCurve2d(ifc4x3Curve);
						return BuildCurve2d(static_cast<IIfcCompositeCurve^>(curve));
					}
					case XCurveType::IfcCompositeCurveOnSurface:
						return BuildCurve2d(static_cast<IIfcCompositeCurveOnSurface^>(curve));
					case XCurveType::IfcEllipse:
						return BuildCurve2d(static_cast<IIfcEllipse^>(curve));
					case XCurveType::IfcIndexedPolyCurve:
						return BuildCurve2d(static_cast<IIfcIndexedPolyCurve^>(curve));
					case XCurveType::IfcLine:
						return BuildCurve2d(static_cast<IIfcLine^>(curve));
					case XCurveType::IfcOffsetCurve2D:
						return BuildCurve2d(static_cast<IIfcOffsetCurve2D^>(curve));
					case XCurveType::IfcPcurve:
						return BuildCurve2d(static_cast<IIfcPcurve^>(curve));
					case XCurveType::IfcPolyline:
						return BuildCurve2d(static_cast<IIfcPolyline^>(curve));
					case XCurveType::IfcRationalBSplineCurveWithKnots:
						return BuildCurve2d(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::IfcSurfaceCurve:
						return BuildCurve2d(static_cast<IIfcSurfaceCurve^>(curve));
					case XCurveType::IfcTrimmedCurve:
						return BuildCurve2d(static_cast<IIfcTrimmedCurve^>(curve));
					case XCurveType::IfcClothoid:
					case XCurveType::IfcCosineSpiral:
					case XCurveType::IfcSineSpiral:
					case XCurveType::IfcSecondOrderPolynomialSpiral:
					case XCurveType::IfcThirdOrderPolynomialSpiral:
					case XCurveType::IfcSeventhOrderPolynomialSpiral:
						throw RaiseGeometryFactoryException("Use BuildSpiral method to build spiral curve types", curve);
					default:
						throw RaiseGeometryFactoryException("Unsupported 2d curve type", curve);
				}
			}

			Handle(Geom2d_BSplineCurve) CurveFactory::BuildCurve2d(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots)
			{

				if (dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(ifcBSplineCurveWithKnots)) return BuildCurve2d(static_cast<IIfcRationalBSplineCurveWithKnots^>(ifcBSplineCurveWithKnots));
				//all control points shall have the same dimensionality of 2
				int numPoles = ifcBSplineCurveWithKnots->ControlPointsList->Count;
				int numKnots = ifcBSplineCurveWithKnots->Knots->Count;

				int numKnotMultiplicities = ifcBSplineCurveWithKnots->KnotMultiplicities->Count;

				if (numKnots != numKnotMultiplicities)
					throw RaiseGeometryFactoryException("Rule CorrespondingKnotLists: The number of elements in the knot multiplicities list shall be equal to the number of elements in the knots list", ifcBSplineCurveWithKnots);

				TColgp_Array1OfPnt2d poles(1, numPoles);
				TColStd_Array1OfReal knots(1, numKnots);
				TColStd_Array1OfInteger knotMultiplicities(1, numKnotMultiplicities);

				int i = 1;
				for each (IIfcCartesianPoint ^ cp in ifcBSplineCurveWithKnots->ControlPointsList)
				{
					if ((int)cp->Dim != 2)
						throw RaiseGeometryFactoryException("Rule SameDim: All control points shall have the same dimensionality.", ifcBSplineCurveWithKnots);
					poles.SetValue(i, gp_Pnt2d(cp->X, cp->Y));
					i++;
				}

				i = 1;
				for each (double knot in ifcBSplineCurveWithKnots->Knots)
				{
					knots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (int multiplicity in ifcBSplineCurveWithKnots->KnotMultiplicities)
				{
					knotMultiplicities.SetValue(i, multiplicity);
					i++;
				}
				Handle(Geom2d_BSplineCurve) bSpline = OccHandle().BuildBSplineCurve2d(poles, knots, knotMultiplicities, (int)ifcBSplineCurveWithKnots->Degree);
				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("Failed to build IfcBSplineCurveWithKnots", ifcBSplineCurveWithKnots);
				return bSpline;
			}

			Handle(Geom2d_Circle) CurveFactory::BuildCurve2d(IIfcCircle^ ifcCircle)
			{
				if (ifcCircle->Radius <= 0)
					throw RaiseGeometryFactoryException("Circle radius cannot be <= 0.", ifcCircle);
				if (2 != (int)ifcCircle->Dim)
					throw RaiseGeometryFactoryException("Cannot build a 2D circle from a 3D circle", ifcCircle);
				IIfcAxis2Placement2D^ axis2d = dynamic_cast<IIfcAxis2Placement2D^>(ifcCircle->Position);
				if (axis2d == nullptr)
					throw RaiseGeometryFactoryException("Cannot build a 2D curve with 3D placement", ifcCircle->Position);
				gp_Ax22d ax22d;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement2d(axis2d, ax22d))
					throw RaiseGeometryFactoryException("Cannot build IIfcAxis2Placement2D, see logs", ifcCircle);
				Handle(Geom2d_Circle) circle2d = OccHandle().BuildCircle2d(ax22d, ifcCircle->Radius);
				if (circle2d.IsNull())
					throw RaiseGeometryFactoryException("Cannot build 2D circle, see logs", ifcCircle);

				return circle2d;
			}

			Handle(Geom2d_BSplineCurve) CurveFactory::BuildCurve2d(IIfcCompositeCurve^ ifcCompositeCurve)
			{
				XCurveType curveType;
				TColGeom2d_SequenceOfBoundedCurve segments;
				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{
					IIfcReparametrisedCompositeCurveSegment^ reparameterisedSegment = dynamic_cast<IIfcReparametrisedCompositeCurveSegment^>(segment);
					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (!IsBoundedCurve(segment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
					Handle(Geom2d_Curve) hSegment = BuildCurve2d(segment->ParentCurve, curveType);
					if (hSegment.IsNull())
						throw RaiseGeometryFactoryException("Composite curve segment is incorrectly defined", segment);
					Handle(Geom2d_BoundedCurve) boundedCurve = Handle(Geom2d_BoundedCurve)::DownCast(hSegment);
					if (boundedCurve.IsNull())
						throw RaiseGeometryFactoryException("Compound curve segments must be bounded curves", segment);
					if (!segment->SameSense)
						boundedCurve->Reverse();
					segments.Append(boundedCurve);
				}

				Handle(Geom2d_BSplineCurve) bSpline = OccHandle().BuildCompositeCurve2d(segments, ModelGeometryService->MinimumGap); //use minimum gap for tolerance to avoid issues with curves and line tolerance errors
				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("Composite curve could not be built", ifcCompositeCurve);
				return bSpline;
			}

			Handle(Geom2d_BSplineCurve) CurveFactory::BuildCurve2d(Ifc4x3::GeometryResource::IfcCompositeCurve^ ifcCompositeCurve)
			{
				TColGeom2d_SequenceOfBoundedCurve segments;
				ProcessCompositeCurveSegments(ifcCompositeCurve, segments);

				BRep_Builder builder;
				TopoDS_Compound occCompound;
				builder.MakeCompound(occCompound);

				Handle(Geom2d_BSplineCurve) bSpline = EXEC_NATIVE->BuildCompositeCurve2d(segments, ModelGeometryService->OneMeter);

				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("Composite curve could not be built", ifcCompositeCurve);

				return bSpline;
			}

			void CurveFactory::ProcessCompositeCurveSegments(Xbim::Ifc4x3::GeometryResource::IfcCompositeCurve^ ifcCompositeCurve, TColGeom2d_SequenceOfBoundedCurve& segments)
			{
				XCurveType curveType;

				for each (Ifc4x3::GeometryResource::IfcSegment ^ segment in ifcCompositeCurve->Segments)
				{

					Ifc4x3::GeometryResource::IfcReparametrisedCompositeCurveSegment^ reparameterisedSegment
						= dynamic_cast<Ifc4x3::GeometryResource::IfcReparametrisedCompositeCurveSegment^>(segment);

					Ifc4x3::GeometryResource::IfcCompositeCurveSegment^ compositeSegment
						= dynamic_cast<Ifc4x3::GeometryResource::IfcCompositeCurveSegment^>(segment);

					Ifc4x3::GeometryResource::IfcCurveSegment^ curveSegment
						= dynamic_cast<Ifc4x3::GeometryResource::IfcCurveSegment^>(segment);


					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (compositeSegment != nullptr && !IsBoundedCurve(compositeSegment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");

					if (compositeSegment != nullptr)
					{
						Handle(Geom2d_Curve) hSegment = BuildCurve2d(compositeSegment->ParentCurve, curveType);
						if (hSegment.IsNull())
							throw RaiseGeometryFactoryException("Composite curve segment is incorrectly defined", segment);
						Handle(Geom2d_BoundedCurve) boundedCurve = Handle(Geom2d_BoundedCurve)::DownCast(hSegment);
						if (boundedCurve.IsNull())
							throw RaiseGeometryFactoryException("Composite curve segments must be bounded curves", segment);
						if (!compositeSegment->SameSense)
							boundedCurve->Reverse();
						segments.Append(boundedCurve);
					}
					else if (curveSegment != nullptr)
					{
						Handle(Geom2d_Curve) seg = BuildCurveSegment2d(curveSegment);
						
						if (!seg) continue;

						IIfcAxis2Placement2D^ axis2 = dynamic_cast<IIfcAxis2Placement2D^>(curveSegment->Placement);

						if (axis2 == nullptr)
							throw RaiseGeometryFactoryException("CurveSegments placement should be IfcAxis2Placement2D", segment);

						Handle(Geom2d_BoundedCurve) boundedCurve = Handle(Geom2d_BoundedCurve)::DownCast(seg);
						if (boundedCurve.IsNull())
							throw RaiseGeometryFactoryException("Composite curve segments must be bounded curves", segment);

						TransformCurveWithLocation(boundedCurve, axis2);
						auto firstParam = boundedCurve->FirstParameter();
						auto lastParam = boundedCurve->LastParameter();
						segments.Append(boundedCurve);
					}
				}
				 
			}

			Handle(Geom2d_BSplineCurve) CurveFactory::BuildCurve2d(IIfcCompositeCurveOnSurface^ ifcCompositeCurve)
			{
				throw RaiseGeometryFactoryException("IIfcCompositeCurveOnSurface is currently not supported", ifcCompositeCurve);

			}

			Handle(Geom2d_Ellipse) CurveFactory::BuildCurve2d(IIfcEllipse^ ifcEllipse)
			{
				if (2 != (int)ifcEllipse->Dim) throw RaiseGeometryFactoryException("Cannot build a 2D curve from a 3D curve", ifcEllipse);
				IIfcAxis2Placement2D^ axis2d = dynamic_cast<IIfcAxis2Placement2D^>(ifcEllipse->Position);
				if (axis2d == nullptr)
					throw RaiseGeometryFactoryException("Cannot build a 2D curve with 3D placement", ifcEllipse->Position);
				gp_Ax22d ax22d;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement2d(axis2d, ax22d))
					throw RaiseGeometryFactoryException("Cannot build IIfcAxis2Placement2D", axis2d);
				Handle(Geom2d_Ellipse) elipse = OccHandle().BuildEllipse2d(ax22d, ifcEllipse->SemiAxis1, ifcEllipse->SemiAxis2);
				if (elipse.IsNull())
					throw RaiseGeometryFactoryException("Cannot build a Elipse", ifcEllipse);
				return elipse;

			}

			Handle(Geom2d_BSplineCurve) CurveFactory::BuildCurve2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve)
			{
				TColGeom2d_SequenceOfBoundedCurve segments;
				BuildIndexPolyCurveSegments2d(ifcIndexedPolyCurve, segments); //this may throw exceptions
				Handle(Geom2d_BSplineCurve) bspline = OccHandle().BuildIndexedPolyCurve2d(segments, ModelGeometryService->MinimumGap);
				if (bspline.IsNull())
					throw RaiseGeometryFactoryException("IIfcIndexedPolyCurve could not be built", ifcIndexedPolyCurve);
				return bspline;
			}

			Handle(Geom2d_LineWithMagnitude) CurveFactory::BuildCurve2d(IIfcLine^ ifcLine)
			{
				if (2 != (int)ifcLine->Dim)
					throw RaiseGeometryFactoryException("Cannot build a 2D curve from a 3D curve", ifcLine);
				gp_Pnt2d origin;
				if (!GEOMETRY_FACTORY->BuildPoint2d(ifcLine->Pnt, origin))
					throw RaiseGeometryFactoryException("Cannot build a 2D point from a 3D point", ifcLine->Pnt);
				gp_Vec2d direction;
				if (!GEOMETRY_FACTORY->BuildDirection2d(ifcLine->Dir->Orientation, direction))
					throw RaiseGeometryFactoryException("Cannot build a 2D direction", ifcLine->Dir->Orientation);
				Handle(Geom2d_LineWithMagnitude) line = OccHandle().BuildLine2d(origin, direction, ifcLine->Dir->Magnitude);
				if (line.IsNull())
					throw RaiseGeometryFactoryException("Cannot build 2D line, see logs", ifcLine);
				return line;
			}

			Handle(Geom2d_OffsetCurve) CurveFactory::BuildCurve2d(IIfcOffsetCurve2D^ ifcOffsetCurve2D)
			{
				XCurveType curveType;
				Handle(Geom2d_Curve) basisCurve = BuildCurve2d(ifcOffsetCurve2D->BasisCurve, curveType); //throws exceptiom
				Handle(Geom2d_OffsetCurve) offsetCurve = OccHandle().BuildOffsetCurve2d(basisCurve, ifcOffsetCurve2D->Distance);
				if (offsetCurve.IsNull())
					throw RaiseGeometryFactoryException("Cannot build offset curve, see logs", ifcOffsetCurve2D);
				return offsetCurve;
			}

			Handle(Geom2d_Curve) CurveFactory::BuildCurve2d(IIfcPcurve^ ifcPcurve)
			{
				throw gcnew NotImplementedException("BuildCurve2d(IIfcPcurve)");
			}

			Handle(Geom2d_Curve) CurveFactory::BuildCurve2d(IIfcPolyline^ ifcPolyline)
			{
				int pointCount = ifcPolyline->Points->Count;
				if (pointCount < 2)
					throw RaiseGeometryFactoryException("IfcPolyline has less than 2 points. It cannot be built", ifcPolyline);

				if (pointCount == 2)
				{
					gp_Pnt2d start = GEOMETRY_FACTORY->BuildPoint2d(ifcPolyline->Points[0]);
					gp_Pnt2d end = GEOMETRY_FACTORY->BuildPoint2d(ifcPolyline->Points[1]);
					if (start.IsEqual(end, ModelGeometryService->Precision))
					{
						LogInformation(ifcPolyline, "IfcPolyline has only 2 identical points. It has been ignored");
						return Handle(Geom2d_Curve)();
					}
					Handle(Geom2d_TrimmedCurve) lineSeg = OccHandle().BuildTrimmedLine2d(start, end);
					if (lineSeg.IsNull())
						throw RaiseGeometryFactoryException("Invalid IfcPolyline definition", ifcPolyline);
					return lineSeg;
				}
				TColgp_Array1OfPnt2d points(1, ifcPolyline->Points->Count);
				GEOMETRY_FACTORY->GetPolylinePoints2d(ifcPolyline, points);

				Handle(Geom2d_BSplineCurve) polyline = OccHandle().BuildPolyline2d(points, ModelGeometryService->Precision);
				if (polyline.IsNull())
					throw RaiseGeometryFactoryException("Failed to build IfcPolyline", ifcPolyline);
				return polyline;
			}

			Handle(Geom2d_BSplineCurve) CurveFactory::BuildCurve2d(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots)
			{
				TColgp_Array1OfPnt2d poles(1, ifcRationalBSplineCurveWithKnots->ControlPointsList->Count);
				TColStd_Array1OfReal knots(1, ifcRationalBSplineCurveWithKnots->Knots->Count);
				TColStd_Array1OfInteger knotMultiplicities(1, ifcRationalBSplineCurveWithKnots->Knots->Count);
				TColStd_Array1OfReal weights(1, ifcRationalBSplineCurveWithKnots->ControlPointsList->Count);
				int i = 1;
				for each (IIfcCartesianPoint ^ cp in ifcRationalBSplineCurveWithKnots->ControlPointsList)
				{
					poles.SetValue(i, gp_Pnt2d(cp->X, cp->Y));
					i++;
				}

				i = 1;
				for each (double knot in ifcRationalBSplineCurveWithKnots->Knots)
				{
					knots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (int multiplicity in ifcRationalBSplineCurveWithKnots->KnotMultiplicities)
				{
					knotMultiplicities.SetValue(i, multiplicity);
					i++;
				}
				i = 1;
				for each (double weight in ifcRationalBSplineCurveWithKnots->Weights)
				{
					weights.SetValue(i, weight);
					i++;
				}

				Handle(Geom2d_BSplineCurve) bspline = OccHandle().BuildRationalBSplineCurve2d(poles, weights, knots, knotMultiplicities, (int)ifcRationalBSplineCurveWithKnots->Degree);
				if (bspline.IsNull())
					throw RaiseGeometryFactoryException("IIfcRationalBSplineCurveWithKnots could not be built.", ifcRationalBSplineCurveWithKnots);
				return bspline;
			}

			Handle(Geom2d_Curve) CurveFactory::BuildCurve2d(IIfcSurfaceCurve^ ifcPolyline)
			{
				throw gcnew NotImplementedException("BuildCurve2d(IfcSurfaceCurve)");
			}

			Handle(Geom2d_TrimmedCurve) CurveFactory::BuildCurve2d(IIfcTrimmedCurve^ ifcTrimmedCurve)
			{


				//Validation
				if (dynamic_cast<IIfcBoundedCurve^>(ifcTrimmedCurve->BasisCurve))
					throw RaiseGeometryFactoryException("Ifc Formal Proposition: NoTrimOfBoundedCurves. Already bounded curves shall not be trimmed.");
				XCurveType curveType;
				Handle(Geom2d_Curve) basisCurve = BuildCurve2d(ifcTrimmedCurve->BasisCurve, curveType);
				if (!basisCurve.IsNull())
				{
					bool isConic = (dynamic_cast<IIfcConic^>(ifcTrimmedCurve->BasisCurve) != nullptr);
					bool isLine = (dynamic_cast<IIfcLine^>(ifcTrimmedCurve->BasisCurve) != nullptr);
					bool isEllipse = (dynamic_cast<IIfcEllipse^>(ifcTrimmedCurve->BasisCurve) != nullptr);
					bool sense = ifcTrimmedCurve->SenseAgreement;
					//get the parametric values
					Xbim::Ifc4::Interfaces::IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

					bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == Xbim::Ifc4::Interfaces::IfcTrimmingPreference::CARTESIAN);

					double u1 = double::NegativeInfinity, u2 = double::PositiveInfinity;
					IIfcCartesianPoint^ cp1 = nullptr;
					IIfcCartesianPoint^ cp2 = nullptr;

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
						gp_Pnt2d p1;
						gp_Pnt2d p2;
						if (!GEOMETRY_FACTORY->BuildPoint2d(cp1, p1))
							throw RaiseGeometryFactoryException("Trim Point1 is not a 2d point", cp1);
						if (!GEOMETRY_FACTORY->BuildPoint2d(cp2, p2))
							throw RaiseGeometryFactoryException("Trim Point2 is not a 2d point", cp1);
						if (!GeomLib_Tool::Parameter(basisCurve, p1, ModelGeometryService->MinimumGap, u1))
							throw RaiseGeometryFactoryException("Trim Point1 is not on the basis curve");
						if (!GeomLib_Tool::Parameter(basisCurve, p2, ModelGeometryService->MinimumGap, u2))
							throw RaiseGeometryFactoryException("Trim Point2 is not on the basis curve");
					}
					else if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //non-compliant
						throw RaiseGeometryFactoryException("Ifc Formal Proposition: TrimValuesConsistent. Either a single value is specified for Trim, or the two trimming values are of different type (point and parameter)");
					else //we prefer to use parameters but need to adjust
					{
						if (isConic)
						{
							u1 *= ModelGeometryService->RadianFactor; //correct to radians
							u2 *= ModelGeometryService->RadianFactor; //correct to radians
						}
					}
					if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //sanity check in case the logic has missed a situtation
						throw RaiseGeometryFactoryException("Error converting Ifc Trim Points");
					if (Math::Abs(u1 - u2) < ModelGeometryService->Precision) //if the parameters are the same trimming will fail if not a conic curve
					{
						if (isConic)
							return Ptr()->BuildTrimmedCurve2d(basisCurve, 0, Math::PI * 2, true); //return a full circle
						else
						{
							LogInformation(ifcTrimmedCurve->BasisCurve, "Parametric Trim Points are equal and will result in an empty curve");
							return Handle(Geom2d_TrimmedCurve)();
						}
					}
					else
						return Ptr()->BuildTrimmedCurve2d(basisCurve, u1, u2, sense);
				}
				else
					throw RaiseGeometryFactoryException("Failed to build Trimmed Basis Curve");
			}

#pragma endregion

			Handle(Geom2d_Curve) CurveFactory::BuildCurveSegment2d(Ifc4x3::GeometryResource::IfcCurveSegment^ segment)
			{
				Ifc4x3::GeometryResource::IfcSpiral^ spiral = dynamic_cast<Ifc4x3::GeometryResource::IfcSpiral^>(segment->ParentCurve);
				Ifc4x3::GeometryResource::IfcPolynomialCurve^ polyCurve = dynamic_cast<Ifc4x3::GeometryResource::IfcPolynomialCurve^>(segment->ParentCurve);
				Ifc4x3::MeasureResource::IfcLengthMeasure^ startLen = dynamic_cast<Ifc4x3::MeasureResource::IfcLengthMeasure^>(segment->SegmentStart);
				Ifc4x3::MeasureResource::IfcLengthMeasure^ curveLength = dynamic_cast<Ifc4x3::MeasureResource::IfcLengthMeasure^>(segment->SegmentLength);

				// Informal Proposition:
				// SegmentStart and SegmentStart shall be of type IfcLengthMeasure
				if (startLen == nullptr || curveLength == nullptr)
					throw RaiseGeometryFactoryException("IfcCurveSegment egmentStart and SegmentEnd should be of type IfcLengthMeasure", segment);

				double startParam = static_cast<double>(startLen->Value);
				double length = static_cast<double>(curveLength->Value);
				double endParam = startParam + length;
				Geom2d_Spiral::IntegrationSteps = (int)((endParam - startParam) / _modelService->OneMeter);

				if (startParam == endParam)
					return nullptr;

				if (spiral) 
				{
					// Spirals are parameterized by arc length, so startParam and endParam correspond to the starting and ending arc length values along the spiral.
					XCurveType spiralType;
					return EXEC_NATIVE->MoveBoundedCurveToOrigin(BuildBoundedSpiral(spiral, startParam, endParam, spiralType));
				}
				else if (polyCurve) 
				{
					return EXEC_NATIVE->MoveBoundedCurveToOrigin(BuildBoundedPolynomialCurve(polyCurve, startParam, endParam));
				}

				XCurveType curveType;
				Handle(Geom2d_Curve) curve = BuildCurve2d(segment->ParentCurve, curveType);

				if (curve.IsNull())
					throw RaiseGeometryFactoryException("CurveSegment parent curve couldn't be built");

				bool sameSense = true;
				if (length < 0) { // negative curve length indicates opposite sense
					sameSense = false;
				}

				if (curve->IsPeriodic())
				{
					IIfcCircle^ circle = dynamic_cast<IIfcCircle^>(segment->ParentCurve);
					IIfcEllipse^ ellipse = dynamic_cast<IIfcEllipse^>(segment->ParentCurve);
					if (circle != nullptr) {
						double r = circle->Radius;
						startParam = startParam / r;
						endParam = endParam / r;
					}
					else if (ellipse)
					{
						throw RaiseGeometryFactoryException("IfcEllipse is not supported as CurveSegment yet");
					}
				}

				Handle(Geom2d_TrimmedCurve) trimmed = EXEC_NATIVE->BuildTrimmedCurve2d(curve, startParam, endParam, sameSense);

				if (trimmed.IsNull())
					throw RaiseGeometryFactoryException("CurveSegment could not be trimmed");
				 
				return EXEC_NATIVE->MoveBoundedCurveToOrigin(trimmed);;
			}
			 
			template <typename IfcType>
			Handle(Geom2d_Curve) CurveFactory::BuildCompositeCurveSegment2d(IfcType ifcCurve, bool sameSense)
			{
				XCurveType curveType;
				Handle(Geom2d_Curve) curve = BuildCurve2d(ifcCurve, curveType);
				if (curve.IsNull()) return curve;
				if (!sameSense) curve->Reverse();
				return curve;
			}

			template <typename IfcType>
			Handle(Geom_Curve) CurveFactory::BuildCompositeCurveSegment3d(IfcType ifcCurve, bool sameSense)
			{
				XCurveType curveType;
				Handle(Geom_Curve) curve = BuildCurve(ifcCurve, curveType);
				if (curve.IsNull()) return curve;
				if (!sameSense) curve->Reverse();
				
				return curve;
			}

			void CurveFactory::BuildPolylineSegments3d(IIfcPolyline^ ifcPolyline, TColGeom_SequenceOfBoundedCurve& segments)
			{
				TColgp_Array1OfPnt points(1, ifcPolyline->Points->Count);
				GEOMETRY_FACTORY->GetPolylinePoints3d(ifcPolyline, points);
				EXEC_NATIVE->Get3dLinearSegments(points, ModelGeometryService->Precision, segments);
			}

			void CurveFactory::BuildPolylineSegments2d(IIfcPolyline^ ifcPolyline, TColGeom2d_SequenceOfBoundedCurve& segments)
			{
				TColgp_Array1OfPnt2d points(1, ifcPolyline->Points->Count);
				GEOMETRY_FACTORY->GetPolylinePoints2d(ifcPolyline, points);
				EXEC_NATIVE->Get2dLinearSegments(points, ModelGeometryService->Precision, segments);
			}

			void CurveFactory::BuildIndexPolyCurveSegments3d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, TColGeom_SequenceOfBoundedCurve& segments)
			{

				IIfcCartesianPointList3D^ pointList3D = dynamic_cast<IIfcCartesianPointList3D^>(ifcIndexedPolyCurve->Points);
				if (pointList3D == nullptr)
					throw RaiseGeometryFactoryException("IIfcIndexedPolyCurve point list is not 3D", ifcIndexedPolyCurve->Points);


				//get a index of all the points
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
						Ifc4::GeometryResource::IfcArcIndex^ arcIndex = dynamic_cast<Ifc4::GeometryResource::IfcArcIndex^>(segment);
						Ifc4::GeometryResource::IfcLineIndex^ lineIndex = dynamic_cast<Ifc4::GeometryResource::IfcLineIndex^>(segment);
						if (arcIndex != nullptr)
						{

							List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)arcIndex->Value;
							if (indices->Count != 3)
								throw RaiseGeometryFactoryException("There should be three indices in an arc index segment", ifcIndexedPolyCurve);
							gp_Pnt start = poles.Value((int)indices[0]);
							gp_Pnt mid = poles.Value((int)indices[1]);
							gp_Pnt end = poles.Value((int)indices[2]);
							Handle(Geom_Circle) circle = OccHandle().BuildCircle3d(start, mid, end);
							if (!circle.IsNull()) //it is a valid arc
							{
								Handle(Geom_TrimmedCurve) arcSegment = OccHandle().BuildTrimmedCurve3d(circle, start, end, ModelGeometryService->MinimumGap);
								if (arcSegment.IsNull())
									throw RaiseGeometryFactoryException("Failed to trim Arc Index segment", ifcIndexedPolyCurve);
								segments.Append(arcSegment);
							}
							else //most likley the three points are in a line it should be treated as a polyline segment according the the docs
							{
								LogInformation(ifcIndexedPolyCurve, "An ArcIndex of an IfcIndexedPolyCurve has been handled as a LineIndex");
								Handle(Geom_TrimmedCurve) lineSegment = OccHandle().BuildTrimmedLine3d(start, end);
								if (lineSegment.IsNull())
									throw RaiseGeometryFactoryException("A LineIndex of an IfcIndexedPolyCurve could not be built", ifcIndexedPolyCurve);
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
								Handle(Geom_TrimmedCurve) lineSegment = OccHandle().BuildTrimmedLine3d(poles.Value((int)indices[p - 1]), poles.Value((int)indices[p]));
								if (lineSegment.IsNull())
									throw RaiseGeometryFactoryException("A line index segment was invalid", ifcIndexedPolyCurve);
								segments.Append(lineSegment);
							}
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
						Handle(Geom_TrimmedCurve) lineSegment = OccHandle().BuildTrimmedLine3d(poles.Value(p), poles.Value(p + 1));
						if (lineSegment.IsNull())
							throw RaiseGeometryFactoryException("A line index segment was invalid", ifcIndexedPolyCurve);
						segments.Append(lineSegment);
					}
				}
			}

			Handle(Geom_Curve) CurveFactory::BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam, XCurveType% curveType)
			{
				//wee need to observe the correct parametric space, conics in radians oother length

				bool sameParams = Math::Abs(endParam - startParam) < ModelGeometryService->Precision;
				if (sameParams && !IsBoundedCurve(curve))
					throw RaiseGeometryFactoryException("DirectrixBounded: If the values for StartParam or EndParam are omited, then the Directrix has to be a bounded or closed curve.");
				if (3 != (int)curve->Dim)
					throw RaiseGeometryFactoryException("DirectrixDim: The Directrix shall be a curve in three dimensional space.");

				Handle(Geom_Curve) geomCurve = BuildCurve(curve, curveType); //throws Exception

				if (geomCurve.IsNull())
					throw RaiseGeometryFactoryException("Directrix is invalid");
				//trimming
				if (startParam != -1 || endParam != -1)
				{
					if (startParam == -1) startParam = geomCurve->FirstParameter();
					if (endParam == -1) endParam = geomCurve->LastParameter();
					if (geomCurve->IsPeriodic())
					{
						//the parameters must be in radians
						if (startParam > 2 * Math::PI || endParam > Math::PI * 2)
							LogInformation(curve, "Directix trims parameters of periodice curves should be in radians");
						Handle(Geom_Curve)  geomCurveTrimmed = new Geom_TrimmedCurve(geomCurve, startParam, endParam);
						if (geomCurve.IsNull())
							throw RaiseGeometryFactoryException("Directrix could not be trimmed");
						return geomCurveTrimmed;
					}
					else
					{
						Handle(Geom_Curve)  geomCurveTrimmed = OccHandle().TrimDirectrix(geomCurve, startParam, endParam, ModelGeometryService->Precision);
						if (geomCurve.IsNull())
							throw RaiseGeometryFactoryException("Directrix could not be trimmed");
						return geomCurveTrimmed;
					}
				}
				return geomCurve;

			}
			
			Handle(Geom_Curve) CurveFactory::BuildDirectrixCurve(IIfcCurve^ curve, Nullable<IfcParameterValue> startParam, Nullable<IfcParameterValue> endParam)
			{
				double start, end;
				if (startParam.HasValue) start = startParam.Value; else start = -1;
				if (endParam.HasValue) end = endParam.Value; else  end = -1;
				XCurveType curveType;
				return BuildDirectrix(curve, start, end, curveType);
			}

			void CurveFactory::BuildCompositeCurveSegments3d(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom_SequenceOfBoundedCurve& segments)
			{
				segments.Clear();
				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{
					IIfcReparametrisedCompositeCurveSegment^ reparameterisedSegment = dynamic_cast<IIfcReparametrisedCompositeCurveSegment^>(segment);
					if (reparameterisedSegment != nullptr && (double)reparameterisedSegment->ParamLength != 1.)
						throw RaiseGeometryFactoryException("IIfcReparametrisedCompositeCurveSegment is currently unsupported", segment);
					if (!IsBoundedCurve(segment->ParentCurve))
						throw RaiseGeometryFactoryException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
					//if the segment is a polyline or an indexedpolycurve we need to add in the individual edge
					auto polylineSegment = dynamic_cast<IIfcPolyline^>(segment->ParentCurve);
					auto indexPolyCurveSegment = dynamic_cast<IIfcIndexedPolyCurve^>(segment->ParentCurve);
					if (polylineSegment != nullptr)
					{
						BuildPolylineSegments3d(polylineSegment, segments);
					}
					else if (indexPolyCurveSegment != nullptr)
					{
						BuildIndexPolyCurveSegments3d(indexPolyCurveSegment, segments);
					}
					else
					{
						Handle(Geom_Curve) hSegment = BuildCompositeCurveSegment3d(segment->ParentCurve, segment->SameSense);
						if (hSegment.IsNull()) continue;//this will throw an exception if badly defined, a zero length segment (IsNull) is tolerated
						Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(hSegment);
						if (boundedCurve.IsNull())
							throw RaiseGeometryFactoryException("Compound curve segments must be bounded curves", segment);
						/*if (!segment->SameSense)
							boundedCurve->Reverse();*/
						segments.Append(boundedCurve);
					}
				}
			}

			Handle(Geom2d_TrimmedCurve) CurveFactory::BuildLinearSegment(const gp_Pnt2d& start, const gp_Pnt2d& end)
			{
				auto segment = EXEC_NATIVE->BuildTrimmedLine2d(start, end);
				if (segment.IsNull())
					throw RaiseGeometryFactoryException("Not a valid linear segment");
				else
					return segment;
			}

			Handle(Geom_TrimmedCurve) CurveFactory::BuildLinearSegment(const gp_Pnt& start, const gp_Pnt& end)
			{
				auto segment = EXEC_NATIVE->BuildTrimmedLine3d(start, end);
				if (segment.IsNull())
					throw RaiseGeometryFactoryException("Not a valid linear segment");
				else
					return segment;
			}

			bool CurveFactory::IsBoundedCurve(IIfcCurve^ curve)
			{
				if (dynamic_cast<IIfcLine^>(curve)) return false;
				if (dynamic_cast<IIfcOffsetCurve3D^>(curve)) return IsBoundedCurve((static_cast<IIfcOffsetCurve3D^>(curve))->BasisCurve);
				if (dynamic_cast<IIfcOffsetCurve2D^>(curve)) return IsBoundedCurve((static_cast<IIfcOffsetCurve2D^>(curve))->BasisCurve);
				if (dynamic_cast<IIfcPcurve^>(curve)) return false; // This is not always the case for Pcurves, if the BasisSurface is bounded, then it is a bounded curve
				if (dynamic_cast<IIfcSurfaceCurve^>(curve)) return false;
				return true;
			}
			
			bool CurveFactory::Tangent2dAt(const Handle(Geom2d_Curve)& curve, double parameter, gp_Pnt2d& pnt2d, gp_Vec2d& tangent)
			{
				return EXEC_NATIVE->Tangent2dAt(curve, parameter, pnt2d, tangent); //throws exception
			}
		}
	}
}
