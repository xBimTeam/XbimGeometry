#pragma once
#include <ShapeFix_Shell.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepCheck_Status.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Line.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include "../XbimProgressMonitor.h"


class XbimNativeApi
{
public:
	static bool FixShell(TopoDS_Shell& shell, double timeOut, std::string& errMsg);
	static bool FixShape(TopoDS_Shape& shape, double timeOut, std::string& errMsg);
	static bool SewShape(TopoDS_Shape& shape, double tolerance, double timeOut, std::string& errMsg);
	static double Length(const Handle(Geom_Curve)& curve);
	static double Length(const Handle(Geom2d_Curve)& curve);
	static bool IsClosed(const TopoDS_Shell& shell);
	static gp_Dir NormalDir(const TopoDS_Wire& wire, bool& isValid);
	static bool RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, std::vector<int> handles, bool closed, double tol);
	static bool RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, bool closed, double tol);
	static gp_Vec NewellsNormal(const TColgp_Array1OfPnt& loop, bool& isPlanar);

private:
	static void AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double& x, double& y, double& z);

};

