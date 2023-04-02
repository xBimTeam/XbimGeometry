#include "CurveFactory.h"
#include "GeometryFactory.h"
#include "SurfaceFactory.h"

#include <GeomLib.hxx>
#include <GeomLib_Tool.hxx>

#include "../BRep/XLine.h"
#include "../BRep/XLine2d.h"
#include "../BRep/XCircle.h"
#include "../BRep/XCircle2d.h"
#include "../BRep/XEllipse.h"
#include "../BRep/XEllipse2d.h"
#include "../BRep/XTrimmedCurve.h"
#include "../BRep/XTrimmedCurve2d.h"
#include "../BRep/XBSplineCurve.h"

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

				int dim = (int)curve->Dim;

				XCurveType curveType;

				if (dim == 2)
				{

					Handle(Geom2d_Curve) hCurve2d = BuildCurve2d(curve, curveType); //this will throw an exception if it fails		
					return BuildXCurve(hCurve2d, curveType);
				}
				else
				{
					Handle(Geom_Curve) hCurve = BuildCurve(curve, curveType);//this will throw an exception if it fails				
					return BuildXCurve(hCurve, curveType);
				}
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



			template <typename IfcType>
			Handle(Geom2d_Curve) CurveFactory::BuildCompositeCurveSegment2d(IfcType ifcCurve, bool sameSense)
			{
				XCurveType curveType;
				Handle(Geom2d_Curve) curve = BuildCurve2d(ifcCurve, curveType);
				if (curve.IsNull()) return curve;
				if (!sameSense) curve->Reverse();
				return curve;
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
					return BuildCurve(static_cast<IIfcCompositeCurve^>(curve));
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

				default:
					throw RaiseGeometryFactoryException("Curve type not implemented.", curve);
				}
				throw;
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
					IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

					bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

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
					return BuildCurve2d(static_cast<IIfcCompositeCurve^>(curve));
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
					IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

					bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

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
				EXEC_NATIVE->Get3dLinearSegments(points, ModelGeometryService->MinimumGap, segments);
			}
			void CurveFactory::BuildPolylineSegments2d(IIfcPolyline^ ifcPolyline, TColGeom2d_SequenceOfBoundedCurve& segments)
			{
				TColgp_Array1OfPnt2d points(1, ifcPolyline->Points->Count);
				GEOMETRY_FACTORY->GetPolylinePoints2d(ifcPolyline, points);
				EXEC_NATIVE->Get2dLinearSegments(points, ModelGeometryService->MinimumGap, segments);
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

			IXCurve^ CurveFactory::BuildDirectrix(IIfcCurve^ curve, Nullable<double> startParam, Nullable<double> endParam)
			{
				double start, end;
				if (startParam.HasValue) start = startParam.Value; else start = -1;
				if (endParam.HasValue) end = endParam.Value; else  end = -1;
				return BuildDirectrix(curve, start, end);
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
				if (dynamic_cast<IIfcPcurve^>(curve)) return false;
				if (dynamic_cast<IIfcSurfaceCurve^>(curve)) return false;
				return true;
			}
			bool CurveFactory::Tangent2dAt(const Handle(Geom2d_Curve)& curve, double parameter, gp_Pnt2d& pnt2d, gp_Vec2d& tangent)
			{
				return EXEC_NATIVE->Tangent2dAt(curve, parameter, pnt2d, tangent); //throws exception
			}
#pragma endregion
		}
	}
}
