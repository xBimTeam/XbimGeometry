#include "NCurveFactory.h"

#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>
#include <GC_MakeArcOfEllipse.hxx>
#include <GCE2d_MakeArcOfCircle.hxx>
#include <Geom2d_Circle.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Elips2d.hxx>
#include <Geom2d_Ellipse.hxx>
#include <GCE2d_MakeArcOfEllipse.hxx>
#include "TColgp_Array1OfPnt2d.hxx"
#include "TColStd_Array1OfReal.hxx"
#include "TColStd_Array1OfInteger.hxx"
#include <Geom2dConvert_CompCurveToBSplineCurve.hxx>
#include <GCE2d_MakeCircle.hxx>
#include <GeomLib_Tool.hxx>
#include <GCE2d_MakeLine.hxx>
#include <BRep_CurveOnSurface.hxx>
#include <Geom2dAPI_PointsToBSpline.hxx>
#include <GC_MakeCircle.hxx>
#include <Geom_OffsetCurve.hxx>
#include <BRep_Tool.hxx>
#include <Extrema_ExtPC.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2d_Transformation.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include "../../BRep/OccExtensions/Curves/Segments/Geom2d_Spiral.h"
#include <NCollection_Vector.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>

TColgp_Array1OfPnt NCurveFactory::GetPointsFromProjectionAndHeightCurves(TColgp_Array1OfPnt& points, Standard_Integer nbPoints, Handle(Geom2d_Curve) projection, Handle(Geom2d_Curve) height)
{

	double projectionParm1 = projection->FirstParameter();
	double projectionParm2 = projection->LastParameter();

	Geom2dAdaptor_Curve adaptorHorizontalProjection(projection, projectionParm1, projectionParm2);
	GCPnts_UniformAbscissa uniformAbscissa(adaptorHorizontalProjection, nbPoints);

	for (Standard_Integer i = 1; i <= nbPoints; ++i) {

		Standard_Real param = uniformAbscissa.Parameter(i);
		gp_Pnt2d p2d;
		projection->D0(param, p2d);
		gp_Pnt2d heightPoint;
		height->D0(param, heightPoint);

		// For the Height Function:
		// Y coord of the curve is the Z ccord of the 3D point
		Standard_Real z = heightPoint.Y();
		gp_Pnt p3d(p2d.X(), p2d.Y(), z);
		points.SetValue(i, p3d);
	}
	return points;
}

TColGeom2d_SequenceOfBoundedCurve NCurveFactory::GetSegmentsSequnce(std::vector<Handle(Geom2d_Curve)> curves, Standard_Real tolerance)
{
	TColGeom2d_SequenceOfBoundedCurve segmentsSequence;
	gp_Pnt2d prevEndPoint;
	bool hasPrevEndPoint = false;

	for (const auto& curve : curves)
	{
		Handle(Geom2d_BoundedCurve) boundedCurve = Handle(Geom2d_BoundedCurve)::DownCast(curve);
		if (boundedCurve.IsNull())
			Standard_Failure::Raise("IfcGradientCurve segments can only be bounded curves");

		gp_Pnt2d startPoint, endPoint;
		boundedCurve->D0(boundedCurve->FirstParameter(), startPoint);
		boundedCurve->D0(boundedCurve->LastParameter(), endPoint);
		if (hasPrevEndPoint)
		{
			if (!prevEndPoint.IsEqual(startPoint, tolerance))
			{
				Handle(Geom2d_TrimmedCurve) gapLine = new Geom2d_TrimmedCurve
				(new Geom2d_Line(prevEndPoint,
					gp_Dir2d(startPoint.X() - prevEndPoint.X(), startPoint.Y() - prevEndPoint.Y())), 0, prevEndPoint.Distance(startPoint));
				segmentsSequence.Append(gapLine);
			}
		}

		segmentsSequence.Append(boundedCurve);
		prevEndPoint = endPoint;
		hasPrevEndPoint = true;
	}
	return segmentsSequence;
}


TColGeom_SequenceOfBoundedCurve NCurveFactory::GetSegmentsSequnce(std::vector<Handle(Geom_Curve)> curves, Standard_Real tolerance) 
{
	TColGeom_SequenceOfBoundedCurve segmentsSequence;
	gp_Pnt prevEndPoint;
	bool hasPrevEndPoint = false;

	for (const auto& curve : curves)
	{
		Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(curve);
		if (boundedCurve.IsNull())
			Standard_Failure::Raise("IfcSegmentedReferenceCurve segments can only be bounded curves");

		gp_Pnt startPoint, endPoint;
		boundedCurve->D0(boundedCurve->FirstParameter(), startPoint);
		boundedCurve->D0(boundedCurve->LastParameter(), endPoint);
		if (hasPrevEndPoint)
		{
			if (!prevEndPoint.IsEqual(startPoint, tolerance))
			{
				Handle(Geom_TrimmedCurve) gapLine = new Geom_TrimmedCurve
				(new Geom_Line(prevEndPoint,
					gp_Dir(startPoint.X() - prevEndPoint.X(), startPoint.Y() - prevEndPoint.Y(), startPoint.Z() - prevEndPoint.Z())), 0, prevEndPoint.Distance(startPoint));
				segmentsSequence.Append(gapLine);
			}
		}

		segmentsSequence.Append(boundedCurve);
		prevEndPoint = endPoint;
		hasPrevEndPoint = true;
	}
	return segmentsSequence;
}



Handle(Geom2d_Curve) NCurveFactory::MoveBoundedCurveToOrigin(const Handle(Geom2d_BoundedCurve)& boundedCurve)
{
	Standard_Real firstParam = boundedCurve->FirstParameter();

	gp_Pnt2d startPoint;
	gp_Vec2d tangent;
	gp_Trsf2d translation;
	gp_Trsf2d rotation;

	boundedCurve->D1(firstParam, startPoint, tangent);
	tangent.Normalize();
	gp_Vec2d translationVec(-startPoint.X(), -startPoint.Y());
	translation.SetTranslation(translationVec);

	Standard_Real angle = atan2(tangent.Y(), tangent.X());
	rotation.SetRotation(gp_Pnt2d(.0, .0),  - angle);

	gp_Trsf2d combinedTransformation = rotation * translation;
	
	boundedCurve->Transform(combinedTransformation);

	return boundedCurve;
}


Handle(Geom2d_Curve) NCurveFactory::AlignToXAxis(const Handle(Geom2d_BoundedCurve)& boundedCurve)
{
	Standard_Real firstParam = boundedCurve->FirstParameter();

	gp_Pnt2d startPoint;
	gp_Vec2d tangent;
	gp_Trsf2d rotation;

	boundedCurve->D1(firstParam, startPoint, tangent);
	Standard_Real angle = atan2(tangent.Y(), tangent.X());
	rotation.SetRotation(gp_Pnt2d(.0, .0), -angle);

	boundedCurve->Transform(rotation);

	return boundedCurve;
}

int NCurveFactory::Intersections(const Handle(Geom2d_Curve)& c1, const Handle(Geom2d_Curve)& c2, TColgp_Array1OfPnt2d& intersections, double intersectTolerance)
{
	try
	{
		Geom2dAPI_InterCurveCurve extrema(c1, c2, intersectTolerance);

		intersections.Resize(1, extrema.NbPoints(), false);
		for (Standard_Integer i = 1; i <= extrema.NbPoints(); i++)
		{
			intersections.SetValue(i, extrema.Point(i));
		}
		return extrema.NbPoints();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid BSplineCurve specification");
		return -1; //return -1 for failure checking
	}
}
Handle(Geom2d_BSplineCurve) NCurveFactory::BuildBSplineCurve2d(const TColgp_Array1OfPnt2d& poles, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree)
{
	try
	{
		return new Geom2d_BSplineCurve(poles, knots, knotMultiplicities, degree);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid BSplineCurve specification");
		return Handle(Geom2d_BSplineCurve)(); //return null handle for checking
	}

}
Handle(Geom_BSplineCurve) NCurveFactory::BuildBSplineCurve3d(const TColgp_Array1OfPnt& poles, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree)
{
	try
	{
		Handle(Geom_BSplineCurve) bspline = new Geom_BSplineCurve(poles, knots, knotMultiplicities, degree);

		return bspline;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid BSplineCurve specification");
		return Handle(Geom_BSplineCurve)(); //return null handle for checking
	}

}

Handle(Geom2d_BSplineCurve) NCurveFactory::BuildRationalBSplineCurve2d(const TColgp_Array1OfPnt2d& poles, const TColStd_Array1OfReal& weights, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree)
{
	try
	{
		return new Geom2d_BSplineCurve(poles, weights, knots, knotMultiplicities, degree);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid Rational BSplineCurve specification");
		return Handle(Geom2d_BSplineCurve)(); //return null handle for checking
	}
}

Handle(Geom2d_Circle) NCurveFactory::BuildCircle2d(const gp_Ax22d& axis, double radius)
{
	try
	{
		return new Geom2d_Circle(axis, radius);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined circle");
		return Handle(Geom2d_Circle)(); //return null handle for checking
	}
}

Handle(Geom2d_Circle) NCurveFactory::BuildCircle2d(const gp_Pnt2d& start, const gp_Pnt2d& mid, const gp_Pnt2d& end)
{
	try
	{
		GCE2d_MakeCircle circleMaker(start, mid, end);
		if (circleMaker.IsDone()) //it is a valid arc
			return circleMaker.Value();
		else
			Standard_Failure::Raise("Circle could not be built from 3 points");
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined circle");

	}
	return Handle(Geom2d_Circle)(); //return null handle for checking
}

Handle(Geom2d_BSplineCurve) NCurveFactory::BuildCompositeCurve2d(const TColGeom2d_SequenceOfBoundedCurve& segments, double tolerance)
{
	try
	{
		Geom2dConvert_CompCurveToBSplineCurve compositeConverter
					(Convert_ParameterisationType::Convert_RationalC1); //provides exact parameterisation


		//all the segments will be bounded curves or offset curves base on a bounded curve
		for (auto& it = segments.cbegin(); it != segments.cend(); ++it)
		{
			Handle(Geom2d_BoundedCurve) curve = *it;
			Handle(Geom2d_Spiral) spiral = Handle(Geom2d_Spiral)::DownCast(curve);

			if (spiral) {
				// Approximate the curve
				Standard_Real firstParam = curve->FirstParameter();
				Standard_Real lastParam = curve->LastParameter();
				int numPoints = spiral.get()->GetIntervalsCount();
				TColgp_Array1OfPnt2d points(1, numPoints);

				for (Standard_Integer i = 1; i <= numPoints; ++i) {
					Standard_Real U = firstParam + (lastParam - firstParam) * (i - 1) / (numPoints - 1);
					gp_Pnt2d P;
					curve->D0(U, P);
					points.SetValue(i, P);
				}

				Geom2dAPI_PointsToBSpline curveBuilder(points, 8, 8, GeomAbs_CN);
				Handle(Geom2d_BSplineCurve) approximate = curveBuilder.Curve();
				if (!compositeConverter.Add(approximate, tolerance, false))
				{
					Standard_Failure::Raise("Compound curve segment is not continuous");
				}
			}
			else
				if (!compositeConverter.Add(curve, tolerance, false))
			{ 
				Standard_Failure::Raise("Compound curve segment is not continuous");
			}
		}
		return compositeConverter.BSplineCurve();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined composite curve");
		return Handle(Geom2d_BSplineCurve)(); //return null handle for checking
	}

}

Handle(Geom2d_BSplineCurve) NCurveFactory::BuildIndexedPolyCurve2d(const TColGeom2d_SequenceOfBoundedCurve& segments, double tolerance)
{

	try
	{
		Geom2dConvert_CompCurveToBSplineCurve converter;
		for (auto&& segment : segments)
		{
			if (!converter.Add(segment, tolerance))
				Standard_Failure::Raise("Could not add segment to IndexPolyCurve2d");
		}
		return converter.BSplineCurve();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid line specification");
		return Handle(Geom2d_BSplineCurve)(); //return null handle for checking
	}
}
Handle(Geom_BSplineCurve) NCurveFactory::BuildIndexedPolyCurve3d(const TColGeom_SequenceOfBoundedCurve& segments, double tolerance)
{
	try
	{
		GeomConvert_CompCurveToBSplineCurve converter;
		for (auto&& segment : segments)
		{
			if (!converter.Add(segment, tolerance))
				Standard_Failure::Raise("Could not add segment to IndexPolyCurve2d");
		}
		return converter.BSplineCurve();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid line specification");
		return Handle(Geom_BSplineCurve)(); //return null handle for checking
	}
}

Handle(Geom2d_LineWithMagnitude) NCurveFactory::BuildLine2d(const gp_Pnt2d& pnt, const gp_Vec2d& dir, double magnitude)
{
	try
	{
		return new Geom2d_LineWithMagnitude(pnt, dir, magnitude);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid line specification");
		return Handle(Geom2d_LineWithMagnitude)(); //return null handle for checking
	}
}

Handle(Geom2d_OffsetCurve) NCurveFactory::BuildOffsetCurve2d(const Handle(Geom2d_Curve)& basisCurve, double offset)
{
	try
	{
		return new Geom2d_OffsetCurve(basisCurve, offset);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined offset curve");
		return Handle(Geom2d_OffsetCurve)(); //return null handle for checking
	}
}




Handle(Geom_LineWithMagnitude) NCurveFactory::BuildLine3d(const gp_Pnt& pnt, const gp_Vec& dir, double magnitude)
{
	try
	{
		return new Geom_LineWithMagnitude(pnt, dir, magnitude);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid line specification");
		return Handle(Geom_LineWithMagnitude)(); //return null handle for checking
	}
}

Handle(Geom_OffsetCurve) NCurveFactory::BuildOffsetCurve3d(const Handle(Geom_Curve)& basisCurve, const gp_Vec& refDir, double offset)
{
	try
	{
		return new Geom_OffsetCurve(basisCurve, offset, refDir);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined offset curve");
		return Handle(Geom_OffsetCurve)(); //return null handle for checking
	}
}

Handle(Geom_Circle) NCurveFactory::BuildCircle3d(const gp_Ax2& axis, double radius)
{
	try
	{
		return new Geom_Circle(axis, radius);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined circle");
		return Handle(Geom_Circle)(); //return null handle for checking
	}
}
Handle(Geom_Circle) NCurveFactory::BuildCircle3d(const gp_Pnt& start, const gp_Pnt& mid, const gp_Pnt& end)
{
	try
	{
		GC_MakeCircle circleMaker(start, mid, end);
		if (circleMaker.IsDone()) //it is a valid arc
			return circleMaker.Value();
		else
			Standard_Failure::Raise("Circle could not be built from 3 points");
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined circle");

	}
	return Handle(Geom_Circle)(); //return null handle for checking
}

Handle(Geom_EllipseWithSemiAxes) NCurveFactory::BuildEllipse3d(const gp_Ax2& axis, double semi1, double semi2)
{
	try
	{
		return new Geom_EllipseWithSemiAxes(axis, semi1, semi2);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined elipse");

	}
	return Handle(Geom_EllipseWithSemiAxes)(); //return null handle for checking
}

Handle(Geom2d_EllipseWithSemiAxes) NCurveFactory::BuildEllipse2d(const gp_Ax22d& axis, double semi1, double semi2)
{
	try
	{
		return new Geom2d_EllipseWithSemiAxes(axis, semi1, semi2);
	}
	catch (const Standard_Failure& e) //only happens with semi axis errors
	{
		LogStandardFailure(e, "Incorrectly defined elipse");

	}
	return Handle(Geom2d_EllipseWithSemiAxes)(); //return null handle for checking
}


Handle(Geom_TrimmedCurve) NCurveFactory::BuildTrimmedCurve3d(const Handle(Geom_Curve)& basisCurve, double u1, double u2, bool sense)
{
	try
	{
		Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(basisCurve);
		if (!circle.IsNull())
		{
			if (!sense)
			{
				double u = u1;
				u1 = u2;
				u2 = u;
			}
			GC_MakeArcOfCircle arcMaker(circle->Circ(), u1, u2, sense);
			if (!arcMaker.IsDone()) Standard_Failure::Raise("Could not build arc segment from circle");
			return arcMaker.Value();
		}
		Handle(Geom_EllipseWithSemiAxes) elipse = Handle(Geom_EllipseWithSemiAxes)::DownCast(basisCurve);
		if (!elipse.IsNull()) //otherwise fall through to end
		{
			u1 = elipse->ConvertIfcTrimParameter(u1);
			u2 = elipse->ConvertIfcTrimParameter(u2);
			GC_MakeArcOfEllipse arcMaker(elipse->Elips(), u1, u2, sense);
			if (!arcMaker.IsDone()) Standard_Failure::Raise("Could not build arc segment from elipse");
			if (!sense) arcMaker.Value()->Reverse(); //need to correct the reverse that has been done to the parameters to make OCC work correctly		
			return arcMaker.Value();
		}
		Handle(Geom_LineWithMagnitude) line = Handle(Geom_LineWithMagnitude)::DownCast(basisCurve);
		if (!line.IsNull()) //otherwise fall through to end
		{
			u1 = line->ConvertIfcTrimParameter(u1);
			u2 = line->ConvertIfcTrimParameter(u2);
			return new Geom_TrimmedCurve(basisCurve, u1, u2, sense, true);
		}

#ifdef _DEBUG
		if (!Handle(Geom_Circle)::DownCast(basisCurve).IsNull() || !Handle(Geom_Ellipse)::DownCast(basisCurve).IsNull() || !Handle(Geom_Line)::DownCast(basisCurve).IsNull())
		{
			Standard_Failure::Raise("OCC Circle, Line and Elipse definitions should not be used for trimming scenarios");
		}
#endif
		return new Geom_TrimmedCurve(basisCurve, u1, u2, sense, true);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined trimmed curve");

	}
	return Handle(Geom_TrimmedCurve)(); //return null handle for checking
}

Handle(Geom_TrimmedCurve) NCurveFactory::BuildTrimmedCurve3d(const Handle(Geom_Curve)& basisCurve, const gp_Pnt& start, const gp_Pnt& end, double tolerance)
{
	try
	{
		double u1, u2;
		GeomLib_Tool::Parameter(basisCurve, start, tolerance, u1);
		GeomLib_Tool::Parameter(basisCurve, end, tolerance, u2);
		return new Geom_TrimmedCurve(basisCurve, u1, u2);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined bounded curve");
		return Handle(Geom_TrimmedCurve)(); //return null handle for checking
	}
}




Handle(Geom_BSplineCurve) NCurveFactory::BuildCompositeCurve3d(const TColGeom_SequenceOfBoundedCurve& segments, double tolerance)
{
	try
	{
		GeomConvert_CompCurveToBSplineCurve compositeConverter(Convert_ParameterisationType::Convert_RationalC1); //provides exact parameterisation

		//all the segments will be bounded curves or offset curves base on a bounded curve
		//we don use the WithRatio option as IFC does not parameterise composite curves this way
		for (auto& it = segments.cbegin(); it != segments.cend(); ++it)
		{
			Handle(Geom_Curve) curve = *it;
			Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(curve);
			if (boundedCurve.IsNull())
				Standard_Failure::Raise("Compound curve segments must be bounded curves");
			if (!compositeConverter.Add(boundedCurve, tolerance, false, false))
				Standard_Failure::Raise("Compound curve segment is not continuous");
		}
		return compositeConverter.BSplineCurve();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined composite curve");
		return Handle(Geom_BSplineCurve)(); //return null handle for checking
	}

}

Handle(Geom2d_BSplineCurve) NCurveFactory::BuildPolyline2d(const TColgp_Array1OfPnt2d& points, double tolerance)
{
	try
	{
		Geom2dConvert_CompCurveToBSplineCurve compositeConverter(Convert_ParameterisationType::Convert_TgtThetaOver2); //provides exact parameterisation for line segs		
		int pointCount = points.Length();
		int lastPointIdx = 1;
		for (Standard_Integer i = 1; i < pointCount; i++)
		{
			const gp_Pnt2d& start = points.Value(lastPointIdx);
			const gp_Pnt2d& end = points.Value(i + 1);
			if (!start.IsEqual(end, tolerance)) //ignore very small segments
			{
				Handle(Geom2d_TrimmedCurve) lineSeg = BuildTrimmedLine2d(start, end);
				//move the lastIndex on
				lastPointIdx++;
				if (!compositeConverter.Add(lineSeg, tolerance, false))
					Standard_Failure::Raise("Polyline segment is not continuous"); //this clearly should never happen
			} //else if we skip a segment because it is small lastPointIdx remains the same
		}
		if (lastPointIdx == 1) //we have failed to add anything
			Standard_Failure::Raise("The Polyline has no segments");
		return compositeConverter.BSplineCurve();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined polyline");
		return Handle(Geom2d_BSplineCurve)(); //return null handle for checking
	}
}

int NCurveFactory::Get3dLinearSegments(const TColgp_Array1OfPnt& points, double tolerance, TColGeom_SequenceOfBoundedCurve& segments)
{
	int pointCount = points.Length();
	int lastPointIdx = 1;
	for (Standard_Integer i = 1; i < pointCount; i++)
	{
		const gp_Pnt& start = points.Value(lastPointIdx);
		const gp_Pnt& end = points.Value(i + 1);
		if (!start.IsEqual(end, tolerance)) //ignore very small segments
		{
			Handle(Geom_TrimmedCurve) lineSeg = BuildTrimmedLine3d(start, end);
			//move the lastIndex on
			lastPointIdx++;
			segments.Append(lineSeg);
		} //else if we skip a segment because it is small lastPointIdx remains the same
	}
	return lastPointIdx - 1;
}

int NCurveFactory::Get2dLinearSegments(const TColgp_Array1OfPnt2d& points, double tolerance, TColGeom2d_SequenceOfBoundedCurve& segments)
{
	int pointCount = points.Length();
	int lastPointIdx = 1;
	for (Standard_Integer i = 1; i < pointCount; i++)
	{
		const gp_Pnt2d& start = points.Value(lastPointIdx);
		const gp_Pnt2d& end = points.Value(i + 1);
		double dist = std::abs(start.Distance(end));
		if (dist > tolerance) //ignore very small segments
		{
			Handle(Geom2d_TrimmedCurve) lineSeg = BuildTrimmedLine2d(start, end);
			//move the lastIndex on
			lastPointIdx++;
			segments.Append(lineSeg);
		}
		else // we skip a segment because it is small lastPointIdx remains the same
		{
			pLoggingService->LogDebug("A segment with a length less than the model tolerance has been ignored");
		}
	}
	return lastPointIdx - 1;
}
Handle(Geom_BSplineCurve) NCurveFactory::BuildPolyline3d(const TColgp_Array1OfPnt& points, double tolerance)
{
	try
	{
		GeomConvert_CompCurveToBSplineCurve compositeConverter(Convert_ParameterisationType::Convert_RationalC1); //provides exact parameterisation for line segs		
		int pointCount = points.Length();
		int lastPointIdx = 1;
		for (Standard_Integer i = 1; i < pointCount; i++)
		{
			const gp_Pnt& start = points.Value(lastPointIdx);
			const gp_Pnt& end = points.Value(i + 1);
			if (!start.IsEqual(end, tolerance)) //ignore very small segments
			{
				Handle(Geom_TrimmedCurve) lineSeg = BuildTrimmedLine3d(start, end);
				//move the lastIndex on
				lastPointIdx++;
				if (!compositeConverter.Add(lineSeg, tolerance, false, true)) //use withratio for better pergormance as we have no curves in the polyline
					Standard_Failure::Raise("Polyline segment is not continuous"); //this clearly should never happen
			} //else if we skip a segment because it is small lastPointIdx remains the same
		}
		if (lastPointIdx == 1) //we have failed to add anything
			Standard_Failure::Raise("The Polyline has no segments");
		return compositeConverter.BSplineCurve();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined polyline");
		return Handle(Geom_BSplineCurve)(); //return null handle for checking
	}
}

Handle(Geom_TrimmedCurve)  NCurveFactory::BuildTrimmedLine3d(const gp_Pnt& start, const gp_Pnt& end)
{
	try
	{
		gp_Vec dir(start, end);
		gp_Lin line(start, dir);
		Handle(Geom_Line) hLine = new Geom_Line(line);
		return new Geom_TrimmedCurve(hLine, 0, dir.Magnitude());
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined bounded line");
		return Handle(Geom_TrimmedCurve)(); //return null handle for checking
	}
}

Handle(Geom2d_TrimmedCurve) NCurveFactory::BuildTrimmedCurve2d(const Handle(Geom2d_Curve)& basisCurve, double u1, double u2, bool sense)
{
	try
	{
		Handle(Geom2d_Circle) circle = Handle(Geom2d_Circle)::DownCast(basisCurve);
		if (!circle.IsNull())
		{
			if (!sense)
			{
				double u = u1;
				u1 = u2;
				u2 = u;
			}

			GCE2d_MakeArcOfCircle arcMaker(circle->Circ2d(), u1, u2, sense);
			if (!arcMaker.IsDone())
				Standard_Failure::Raise("Could not build arc segment from circle");
			return arcMaker.Value();
		}
		Handle(Geom2d_EllipseWithSemiAxes) elipse = Handle(Geom2d_EllipseWithSemiAxes)::DownCast(basisCurve);
		if (!elipse.IsNull()) //otherwise fall through to end
		{
			u1 = elipse->ConvertIfcTrimParameter(u1);
			u2 = elipse->ConvertIfcTrimParameter(u2);
			GCE2d_MakeArcOfEllipse arcMaker(elipse->Elips2d(), u1, u2, sense);
			if (!arcMaker.IsDone())
				Standard_Failure::Raise("Could not build arc segment from elipse");
			if (!sense) arcMaker.Value()->Reverse(); //need to correct the reverse that has been done to the parameters to make OCC work correctly		
			return arcMaker.Value();
		}
		Handle(Geom2d_LineWithMagnitude) line = Handle(Geom2d_LineWithMagnitude)::DownCast(basisCurve);
		if (!line.IsNull()) //otherwise fall through to end
		{
			u1 = line->ConvertIfcTrimParameter(u1);
			u2 = line->ConvertIfcTrimParameter(u2);
			return new Geom2d_TrimmedCurve(basisCurve, u1, u2, sense, true);
		}
#ifdef _DEBUG
		if (!Handle(Geom2d_Circle)::DownCast(basisCurve).IsNull() || !Handle(Geom2d_Ellipse)::DownCast(basisCurve).IsNull() || !Handle(Geom2d_Line)::DownCast(basisCurve).IsNull())
		{
			Standard_Failure::Raise("OCC Circle, line and Elipse definitions should not be used for trimming scenarios");
		}
#endif
		Handle(Geom2d_TrimmedCurve) trimmedCurve = new Geom2d_TrimmedCurve(basisCurve, u1, u2, sense, true);
		return trimmedCurve;
	}

	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined trimmed curve");
		return Handle(Geom2d_TrimmedCurve)(); //return null handle for checking
	}
}

Handle(Geom2d_TrimmedCurve) NCurveFactory::BuildTrimmedLine2d(const gp_Pnt2d& start, const gp_Pnt2d& end)
{
	try
	{
		GCE2d_MakeLine lineMaker(start, end);
		if (!lineMaker.IsDone())
			Standard_Failure::Raise("Failed to build line betwen two points");
		return new Geom2d_TrimmedCurve(lineMaker.Value(), 0, start.Distance(end));
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid line specification");
		return Handle(Geom2d_TrimmedCurve)(); //return null handle for checking
	}
}

Handle(Geom2d_TrimmedCurve) NCurveFactory::BuildTrimmedCurve2d(const Handle(Geom2d_Curve)& basisCurve, const gp_Pnt2d& start, const gp_Pnt2d& end, double tolerance)
{
	try
	{
		double u1, u2;
		GeomLib_Tool::Parameter(basisCurve, start, tolerance, u1);
		GeomLib_Tool::Parameter(basisCurve, end, tolerance, u2);
		return new Geom2d_TrimmedCurve(basisCurve, u1, u2);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined bounded curve");
		return Handle(Geom2d_TrimmedCurve)(); //return null handle for checking
	}
}

Handle(Geom_BSplineCurve) NCurveFactory::BuildRationalBSplineCurve3d(const TColgp_Array1OfPnt& poles, const TColStd_Array1OfReal& weights, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree)
{
	try
	{
		return new Geom_BSplineCurve(poles, weights, knots, knotMultiplicities, degree);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid Rational BSplineCurve specification");
		return Handle(Geom_BSplineCurve)(); //return null handle for checking
	}
}

Handle(Geom_Curve) NCurveFactory::TrimDirectrix(const Handle(Geom_Curve)& basisCurve, double u1, double u2, double precision)
{
	try
	{
		Handle(Geom_BSplineCurve) bspline = Handle(Geom_BSplineCurve)::DownCast(basisCurve);
		if (!bspline.IsNull())
		{
			//make a copy
			Handle(Geom_BSplineCurve) bsplineCopy = Handle(Geom_BSplineCurve)::DownCast(bspline->Copy());
			bsplineCopy->Segment(u1, u2, precision);
			return bsplineCopy;
		}
		else
		{
			return new Geom_TrimmedCurve(basisCurve, u1, u2);
		}
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Incorrectly defined trimmed directrix");
		return Handle(Geom_Curve)();
	}


}

Handle(Geom_Curve) NCurveFactory::GetBasisCurve(const Handle(Geom_Curve)& geomCurve)
{
	// kill trimmed curves
	Handle(Geom_Curve) basisCurve = geomCurve;
	Handle(Geom_TrimmedCurve) trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(basisCurve);
	while (!trimmedCurve.IsNull())
	{
		basisCurve = trimmedCurve->BasisCurve();
		trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(basisCurve);
	}
	return basisCurve;
}

bool NCurveFactory::LocateVertexOnCurve(const Handle(Geom_Curve)& geomCurve, const TopoDS_Vertex& V, double maxTolerance, double& parameter, double& actualDistance)
{
	Standard_Real Eps2 = maxTolerance * maxTolerance;
	Handle(Geom_Curve) basisCurve = GetBasisCurve(geomCurve);
	gp_Pnt P = BRep_Tool::Pnt(V);
	GeomAdaptor_Curve GAC(basisCurve);
	Standard_Real D1, D2;
	gp_Pnt P1, P2;
	P1 = GAC.Value(GAC.FirstParameter());
	P2 = GAC.Value(GAC.LastParameter());
	D1 = P1.SquareDistance(P);
	D2 = P2.SquareDistance(P);
	if ((D1 < D2) && (D1 <= Eps2))
	{
		parameter = GAC.FirstParameter();
		actualDistance = sqrt(D1);
		return Standard_True;
	}
	else if ((D2 < D1) && (D2 <= Eps2))
	{
		parameter = GAC.LastParameter();
		actualDistance = sqrt(D2);
		return Standard_True;
	}

	Extrema_ExtPC extrema(P, GAC);
	if (extrema.IsDone())
	{
		Standard_Integer i, index = 0, n = extrema.NbExt();
		Standard_Real Dist2 = RealLast(), dist2min;

		for (i = 1; i <= n; i++) {
			dist2min = extrema.SquareDistance(i);
			if (dist2min < Dist2) {
				index = i;
				Dist2 = dist2min;
			}
		}

		if (index != 0) {
			if (Dist2 <= Eps2) {
				parameter = (extrema.Point(index)).Parameter();
				actualDistance = sqrt(Dist2);
				return Standard_True;
			}
		}
	}
	return Standard_False;
}

bool NCurveFactory::Tangent2dAt(const Handle(Geom2d_Curve)& curve, double parameter, gp_Pnt2d& pnt2d, gp_Vec2d& tangent)
{
	try
	{
		curve->D1(parameter, pnt2d, tangent);
		return true;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Invalid tangent at point on curve");
		return false;
	}



}


bool NCurveFactory::GetPlaneFromWire(const TopoDS_Wire& wire, gp_Pln& plane) {
	BRepBuilderAPI_MakeFace faceMaker(wire);
	if (!faceMaker.IsDone()) {
		return false;
	} 
	Handle(Geom_Surface) surface = BRep_Tool::Surface(faceMaker.Face());
	Handle(Geom_Plane) geomPlane = Handle(Geom_Plane)::DownCast(surface);
	if (geomPlane.IsNull()) {
		return false;
	}

	plane = geomPlane->Pln();
	return true;
}

bool NCurveFactory::GetPlaneFromFace(const TopoDS_Face& face, gp_Pln& plane)
{
 
	Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
	Handle(Geom_Plane) geomPlane = Handle(Geom_Plane)::DownCast(surface);
	if (geomPlane.IsNull()) {
		return false;
	}

	plane = geomPlane->Pln();
	return true;
}


Handle(Geom_Curve) NCurveFactory::TrimCurveByWires(const Handle(Geom_Curve)& curve, const TopoDS_Wire& wire1, const TopoDS_Wire& wire2) {
	std::vector<Standard_Real> parameters;
	for (const TopoDS_Wire& wire : { wire1, wire2 }) {
		gp_Pln plane;
		if (GetPlaneFromWire(wire, plane)) {
			// Intersect the curve with the plane
			Handle(Geom_Surface) geomPlane = new Geom_Plane(plane);
			GeomAPI_IntCS intersector(curve, geomPlane);

			if (intersector.NbPoints() > 0) {
				Standard_Real param;
				Standard_Real v;
				Standard_Real w;
				intersector.Parameters(1, param, v, w);
				parameters.push_back(param);
			}
		}
	}

	if (parameters.size() == 2) {
		if (parameters[0] > parameters[1]) {
			std::swap(parameters[0], parameters[1]);
		}

		Handle(Geom_TrimmedCurve) trimmedCurve = new Geom_TrimmedCurve(curve, parameters[0], parameters[1]);

		return trimmedCurve;
	}

	return curve;
}


Handle(Geom_Curve) NCurveFactory::TrimCurveByFaces(const Handle(Geom_Curve)& curve, const TopoDS_Face& face1, const TopoDS_Face& face2)
{
	std::vector<Standard_Real> parameters;

	// Process each face
	GProp_GProps props;

	BRepGProp::SurfaceProperties(face1, props);
	gp_Pnt facePoint = props.CentreOfMass();

	GeomAPI_ProjectPointOnCurve projector1(facePoint, curve);
	if (projector1.NbPoints() > 0)
	{
		Standard_Real param = projector1.LowerDistanceParameter();
		parameters.push_back(param);
	}


	BRepGProp::SurfaceProperties(face2, props);
	facePoint = props.CentreOfMass();

	GeomAPI_ProjectPointOnCurve projector2(facePoint, curve);
	if (projector2.NbPoints() > 0)
	{
		Standard_Real param = projector2.LowerDistanceParameter();
		parameters.push_back(param);
	}

	if (parameters.size() == 2)
	{
		if (parameters[0] > parameters[1]) {
			std::swap(parameters[0], parameters[1]);
		}

		Handle(Geom_TrimmedCurve) trimmedCurve = new Geom_TrimmedCurve(curve, parameters[0], parameters[1]);
		return trimmedCurve;
	}

	return curve;
}




