#pragma once
#include <gp_Pnt2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Dir2d.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pnt.hxx>
#include <gp_XY.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <BRepBndLib.hxx>
struct LineSegment2d
{
	gp_XY A;
	gp_XY B;
	TopoDS_Edge Edge;
	LineSegment2d() {};

	LineSegment2d(gp_XY a, gp_XY b) : A(a), B(b)
	{
	}

	LineSegment2d(const TopoDS_Edge& edge)
	{
		TopoDS_Vertex vFirst, vLast;
		TopExp::Vertices(edge, vFirst, vLast, true);
		gp_Pnt pFirst = BRep_Tool::Pnt(vFirst);
		gp_Pnt pLast = BRep_Tool::Pnt(vLast);
		A = gp_XY(pFirst.X(), pFirst.Y());
		B = gp_XY(pLast.X(), pLast.Y());
		Edge = edge;
	}
	gp_XY LeftMost()
	{
		Bnd_Box box;
		BRepBndLib::AddClose(Edge, box);
		Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
		box.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
		return gp_XY(srXmin, srYmin);
	}
	/*static GetSegments(const TopoDS_Edge& edge, double linearDeflection, double angularDeflection)
	{
		Handle(Geom2d_Curve) curve;
		Handle(Geom_Surface) surface;
		TopLoc_Location location;
		double first, last;
		BRep_Tool::CurveOnSurface(edge, curve, surface, location, first, last);

	}

	static GetSegments(const TopoDS_Edge& edge, double linearDeflection)
	{

	}*/
	
};
