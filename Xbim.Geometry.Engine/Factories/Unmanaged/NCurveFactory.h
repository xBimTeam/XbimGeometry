#pragma once
#include "NFactoryBase.h"
#include <gp_Pnt.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
#include "../../BRep/OccExtensions/Geom2d_LineWithMagnitude.h"
#include "../../BRep/OccExtensions/Geom_LineWithMagnitude.h"
#include "../../BRep/OccExtensions/Geom_EllipseWithSemiAxes.h"
#include "../../BRep/OccExtensions/Geom2d_EllipseWithSemiAxes.h"
#include <TColGeom2d_SequenceOfCurve.hxx>
#include <TColGeom2d_SequenceOfBoundedCurve.hxx>
#include <Geom2d_OffsetCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <TColGeom_SequenceOfBoundedCurve.hxx>
#include <TopoDS_Vertex.hxx>
public class NCurveFactory : public NFactoryBase
{

public:

#pragma region Geometric methods

	int Intersections(const Handle(Geom2d_Curve)& c1, const Handle(Geom2d_Curve)& c2, TColgp_Array1OfPnt2d& intersections, double intersectTolerance);

	Handle(Geom2d_BSplineCurve) BuildBSplineCurve2d(const TColgp_Array1OfPnt2d& poles, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree);

	Handle(Geom2d_Circle) BuildCircle2d(const gp_Ax22d &axis, double radius);
	Handle(Geom2d_Circle) BuildCircle2d(const gp_Pnt2d &start, const gp_Pnt2d& mid, const gp_Pnt2d& end);
	Handle(Geom2d_BSplineCurve) BuildCompositeCurve2d(const TColGeom2d_SequenceOfBoundedCurve& segments, double tolerance);
	Handle(Geom2d_EllipseWithSemiAxes) BuildEllipse2d(const gp_Ax22d& axis, double major, double minor);
	Handle(Geom2d_BSplineCurve) BuildIndexedPolyCurve2d(const TColGeom2d_SequenceOfBoundedCurve& segments, double tolerance);
	Handle(Geom2d_LineWithMagnitude) BuildLine2d(const gp_Pnt2d& pnt, const gp_Vec2d& dir, double magnitude);
	Handle(Geom2d_OffsetCurve) BuildOffsetCurve2d(const Handle(Geom2d_Curve)& basisCurve, double offset);
	Handle(Geom2d_BSplineCurve) BuildPolyline2d(const TColgp_Array1OfPnt2d& points, double tolerance);
	Handle(Geom2d_BSplineCurve) BuildRationalBSplineCurve2d(const TColgp_Array1OfPnt2d& poles, const TColStd_Array1OfReal& weights, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree);


	Handle(Geom2d_TrimmedCurve) BuildTrimmedCurve2d(const Handle(Geom2d_Curve)& basisCurve, double u1, double u2, bool sense);
	Handle(Geom2d_TrimmedCurve) BuildTrimmedLine2d(const gp_Pnt2d& start, const gp_Pnt2d& end);
	Handle(Geom2d_TrimmedCurve) BuildTrimmedCurve2d(const Handle(Geom2d_Curve)& basisCurve, const gp_Pnt2d& start, const gp_Pnt2d& end, double tolerance);


	Handle(Geom_BSplineCurve) BuildBSplineCurve3d(const TColgp_Array1OfPnt& poles, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree);
	Handle(Geom_Circle) BuildCircle3d(const gp_Ax2& axis, double radius);
	Handle(Geom_Circle) BuildCircle3d(const gp_Pnt& start, const gp_Pnt& mid, const gp_Pnt& end);
	Handle(Geom_BSplineCurve) BuildCompositeCurve3d(const TColGeom_SequenceOfBoundedCurve& segments, double tolerance);
	Handle(Geom_EllipseWithSemiAxes) BuildEllipse3d(const gp_Ax2& axis, double major, double minor);
	Handle(Geom_BSplineCurve) BuildIndexedPolyCurve3d(const TColGeom_SequenceOfBoundedCurve& segments, double tolerance);
	Handle(Geom_LineWithMagnitude) BuildLine3d(const gp_Pnt& pnt, const gp_Vec& dir, double magnitude);
	Handle(Geom_OffsetCurve) BuildOffsetCurve3d(const Handle(Geom_Curve)& basisCurve, const gp_Vec& refDir, double offset);
	Handle(Geom_BSplineCurve) BuildPolyline3d(const TColgp_Array1OfPnt& points, double tolerance);
	Handle(Geom_TrimmedCurve) BuildTrimmedLine3d(const gp_Pnt& start, const gp_Pnt& end);

	Handle(Geom_TrimmedCurve) BuildTrimmedCurve3d(const Handle(Geom_Curve)& basisCurve, double u1, double u2, bool sense);
	Handle(Geom_TrimmedCurve) BuildTrimmedCurve3d(const Handle(Geom_Curve)& basisCurve, const gp_Pnt& start, const gp_Pnt& end, double tolerance);



	Handle(Geom_BSplineCurve) BuildRationalBSplineCurve3d(const TColgp_Array1OfPnt& poles, const TColStd_Array1OfReal& weights, const TColStd_Array1OfReal& knots, const TColStd_Array1OfInteger& knotMultiplicities, int degree);


	Handle(Geom_Curve) TrimDirectrix(const Handle(Geom_Curve)& basisCurve, double u1, double u2, double precision);

	/// <summary>
	/// If a vertex lies on a curve within the maxTolerance, the parameter of the vertex on the curve is returned and the actual distance between the point of the vertex and the curve is calculated
	/// </summary>
	static bool LocateVertexOnCurve(const Handle(Geom_Curve)& C, const TopoDS_Vertex& V, double maxTolerance, double& parameter, double& actualDistance);
	bool Tangent2dAt(const Handle(Geom2d_Curve)& curve, double parameter, gp_Pnt2d& pnt2d, gp_Vec2d& tangent);
#pragma endregion

};




