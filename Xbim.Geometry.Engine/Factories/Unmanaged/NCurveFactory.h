#pragma once
#include "../../Services//Unmanaged/NLoggingService.h"
#include <gp_Pnt.hxx>

#include <Geom_Circle.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>

#include "../../BRep/OccExtensions/Geom2d_LineWithMagnitude.h"
#include "../../BRep/OccExtensions/Geom_LineWithMagnitude.h"
#include "../../BRep/OccExtensions/Geom_EllipseWithSemiAxes.h"
#include "../../BRep/OccExtensions/Geom2d_EllipseWithSemiAxes.h"

#include <TopoDS_Edge.hxx>
#include <TColGeom_SequenceOfCurve.hxx>


class NCurveFactory
{
private:
	NLoggingService* pLoggingService;
public:

	
	NCurveFactory()
	{
		pLoggingService = nullptr;
	};
	~NCurveFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
#pragma region Geometric methods


	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	Handle(Geom2d_LineWithMagnitude) BuildLine2d(gp_Pnt2d pnt, gp_Dir2d dir, double magnitude);
	Handle(Geom_LineWithMagnitude) BuildLine3d(gp_Pnt pnt, gp_Dir dir, double magnitude);

	Handle(Geom_Circle) BuildCircle3d(gp_Ax2 axis, double radius);
	Handle(Geom2d_Circle) BuildCircle2d(gp_Ax2d axis, double radius);

	Handle(Geom_EllipseWithSemiAxes) BuildEllipse3d(gp_Ax2 axis, double major, double minor);
	Handle(Geom2d_EllipseWithSemiAxes) BuildEllipse2d(gp_Ax22d axis, double major, double minor);

	Handle(Geom2d_TrimmedCurve) BuildTrimmedCurve2d(Handle(Geom2d_Curve) basisCurve, double u1, double u2, bool sense);
	Handle(Geom_TrimmedCurve) BuildTrimmedCurve3d(Handle(Geom_Curve) basisCurve, double u1, double u2, bool sense);

	Handle(Geom_BSplineCurve) BuildCompositeCurve(const TColGeom_SequenceOfCurve& segments, double tolerance);

	Handle(Geom_BSplineCurve) BuildPolyline(const TColgp_Array1OfPnt& points, double tolerance);
	Handle(Geom_TrimmedCurve)  BuildBoundedLine3d(const gp_Pnt& start, const gp_Pnt& end);

	Handle(Geom_Curve) TrimDirectrix(Handle(Geom_Curve) basisCurve, double u1, double u2, double precision);
#pragma endregion



};
			



