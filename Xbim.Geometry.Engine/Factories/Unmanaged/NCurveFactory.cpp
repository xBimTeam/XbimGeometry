#include "NCurveFactory.h"
#include <Standard_CString.hxx>
#include <stdio.h>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
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

Handle(Geom2d_Circle) NCurveFactory::BuildCircle2d(gp_Ax2d axis, double radius)
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
			if (!arcMaker.IsDone()) throw Standard_Failure("Could not build arc segment from circle");
			return arcMaker.Value();			
		}
		Handle(Geom_Ellipse) elipse = Handle(Geom_Ellipse)::DownCast(basisCurve);
		if (!elipse.IsNull()) //otherwise fall through to end
		{
			if (!sense)
			{
				Handle(Geom_Ellipse) elipse = Handle(Geom_Ellipse)::DownCast(basisCurve);
				if (!elipse.IsNull()) //otherwise fall through to end
				{
					basisCurve->Reverse();
					Handle(Geom_TrimmedCurve) tc = new Geom_TrimmedCurve(basisCurve, u1, u2, true, true);
					tc->BasisCurve()->Reverse();
					return tc;
				}
			}
		}
		
		return new Geom_TrimmedCurve(basisCurve, u1, u2, sense, true);
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogError(e.GetMessageString());
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
		for (auto it = segments.cbegin(); it != segments.cend(); ++it)
		{
			Handle(Geom_Curve) curve = *it;
			Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(curve);
			if (boundedCurve.IsNull())
				throw Standard_Failure("Compound curve segments must be bounded curves");
			if (!compositeConverter.Add(boundedCurve, tolerance,false, false))
				throw Standard_Failure("Compound curve segment is not continuous");
		}
		return compositeConverter.BSplineCurve();
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Could not build composite curve");
	return Handle(Geom_BSplineCurve)(); //return null handle for checking

}

Handle(Geom2d_TrimmedCurve) NCurveFactory::BuildTrimmedCurve2d(Handle(Geom2d_Curve) basisCurve, double u1, double u2, bool sense)
{
	try
	{
		return new Geom2d_TrimmedCurve(basisCurve, u1, u2, sense);
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogError(e.GetMessageString());
		return Handle(Geom2d_TrimmedCurve)(); //return null handle for checking
	}
}


