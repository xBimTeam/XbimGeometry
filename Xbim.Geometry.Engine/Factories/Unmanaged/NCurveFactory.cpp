#include "NCurveFactory.h"
#include <Standard_CString.hxx>
#include <stdio.h>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>
#include <GC_MakeArcOfEllipse.hxx>

Handle(Geom_LineWithMagnitude) NCurveFactory::BuildLine3d(gp_Pnt pnt, gp_Dir dir, double magnitude)
{
	try
	{
		return new Geom_LineWithMagnitude(pnt, dir, magnitude);
	}
	catch (Standard_Failure)
	{
		char msg[256];
		sprintf_s(msg, "Invalid line direction, (%f , %f, %f)", dir.X(), dir.Y(), dir.Z());
		pLoggingService->LogError(msg);
		return Handle(Geom_LineWithMagnitude)(); //return null handle for checking
	}
}

Handle(Geom_Circle) NCurveFactory::BuildCircle3d(gp_Ax2 axis, double radius)
{
	try
	{
		return new Geom_Circle(axis, radius);
	}
	catch (Standard_Failure)
	{
		pLoggingService->LogError("Incorrectly defined circle");
		return Handle(Geom_Circle)(); //return null handle for checking
	}
}

Handle(Geom2d_Circle) NCurveFactory::BuildCircle2d(gp_Ax22d axis, double radius)
{
	try
	{
		return new Geom2d_Circle(axis, radius);
	}
	catch (Standard_Failure)
	{
		pLoggingService->LogError("Incorrectly defined circle");
		return Handle(Geom2d_Circle)(); //return null handle for checking
	}
}

Handle(Geom_EllipseWithSemiAxes) NCurveFactory::BuildEllipse3d(gp_Ax2 axis, double semi1, double semi2)
{
	try
	{

		return new Geom_EllipseWithSemiAxes(axis, semi1, semi2);
	}
	catch (Standard_Failure)
	{
		char msg[256];
		sprintf_s(msg, "Non-Compliant Semi Axis values, %f , %f", semi1, semi2);
		pLoggingService->LogError(msg);
		return Handle(Geom_EllipseWithSemiAxes)(); //return null handle for checking
	}
}

Handle(Geom2d_EllipseWithSemiAxes) NCurveFactory::BuildEllipse2d(gp_Ax22d axis, double semi1, double semi2)
{
	try
	{
		return new Geom2d_EllipseWithSemiAxes(axis, semi1, semi2);
	}
	catch (Standard_Failure) //only happens with semi axis errors
	{
		char msg[256];
		sprintf_s(msg, "Non-Compliant Semi Axis values, %f , %f", semi1, semi2);
		pLoggingService->LogError(msg);
		return Handle(Geom2d_EllipseWithSemiAxes)(); //return null handle for checking
	}
}

Handle(Geom2d_LineWithMagnitude) NCurveFactory::BuildLine2d(gp_Pnt2d pnt, gp_Dir2d dir, double magnitude)
{
	try
	{
		return new Geom2d_LineWithMagnitude(pnt, dir, magnitude);
	}
	catch (Standard_Failure)
	{
		char msg[256];
		sprintf_s(msg, "Invalid line direction, (%f , %f)", dir.X(), dir.Y());
		pLoggingService->LogError(msg);
		return Handle(Geom2d_LineWithMagnitude)(); //return null handle for checking
	}
}
Handle(Geom_TrimmedCurve) NCurveFactory::BuildTrimmedCurve3d(Handle(Geom_Curve) basisCurve, double u1, double u2, bool sense)
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
		Handle(Geom_Ellipse) elipse = Handle(Geom_Ellipse)::DownCast(basisCurve);
		if (!elipse.IsNull()) //otherwise fall through to end
		{
			if (!sense)
			{

				basisCurve->Reverse();
				Handle(Geom_TrimmedCurve) tc = new Geom_TrimmedCurve(basisCurve, u1, u2, true, true);
				tc->BasisCurve()->Reverse();
				return tc;

			}
		}

		return new Geom_TrimmedCurve(basisCurve, u1, u2, sense, true);
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		return Handle(Geom_TrimmedCurve)(); //return null handle for checking
	}
}
Handle(Geom_BSplineCurve) NCurveFactory::BuildCompositeCurve(const TColGeom_SequenceOfCurve& segments, double tolerance)
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
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Could not build composite curve");
	return Handle(Geom_BSplineCurve)(); //return null handle for checking

}

Handle(Geom_BSplineCurve) NCurveFactory::BuildPolyline(const TColgp_Array1OfPnt& points, double tolerance)
{
	try
	{
		GeomConvert_CompCurveToBSplineCurve compositeConverter(Convert_ParameterisationType::Convert_TgtThetaOver2); //provides exact parameterisation for line segs		
		int pointCount = points.Length();
		int lastPointIdx = 1;
		for (Standard_Integer i = 1; i < pointCount; i++)
		{
			const gp_Pnt& start = points.Value(lastPointIdx);
			const gp_Pnt& end = points.Value(i + 1);
			if (!start.IsEqual(end, tolerance)) //ignore very small segments
			{
				Handle(Geom_TrimmedCurve) lineSeg = BuildBoundedLine3d(start, end);
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
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		return Handle(Geom_BSplineCurve)(); //return null handle for checking
	}
}

Handle(Geom_TrimmedCurve)  NCurveFactory::BuildBoundedLine3d(const gp_Pnt& start, const gp_Pnt& end)
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
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		return Handle(Geom_TrimmedCurve)(); //return null handle for checking
	}
}

Handle(Geom2d_TrimmedCurve) NCurveFactory::BuildTrimmedCurve2d(Handle(Geom2d_Curve) basisCurve, double u1, double u2, bool sense)
{
	try
	{
		return new Geom2d_TrimmedCurve(basisCurve, u1, u2, sense);
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		return Handle(Geom2d_TrimmedCurve)(); //return null handle for checking
	}
}

Handle(Geom_Curve) NCurveFactory::TrimDirectrix(Handle(Geom_Curve) basisCurve, double u1, double u2, double precision)
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
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		char msg[256];
		sprintf_s(msg, "Non-Compliant Trim parameters values, %f , %f", u1, u2);
		pLoggingService->LogError(msg);
		return basisCurve; //return original cirve
	}
}


