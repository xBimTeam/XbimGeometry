#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Compound.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Ax22d.hxx>
class NProfileFactory : public NFactoryBase
{
private:
	Handle(Geom_Plane) _xyPlane;
public:
	NProfileFactory()
	{
		_xyPlane = new Geom_Plane(gp::XOY());
	}

	TopoDS_Compound MakeCompound(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
	//Build a face and an outer bound, as profile defs are 2d  the xy plane is assumed for the surface
	TopoDS_Face MakeFace(const TopoDS_Wire& wire);
	TopoDS_Edge MakeEdge(const gp_Pnt& start, const gp_Pnt& end);
	TopoDS_Edge MakeEdge(const gp_Pnt2d& start, const gp_Pnt2d& end);
	TopoDS_Edge MakeEdge(const Handle(Geom_Curve)& hCurve);
	TopoDS_Edge MakeEdge(const Handle(Geom2d_Curve)& hCurve2d);
	TopoDS_Wire MakeWire(const TopoDS_Edge& edge);
	TopoDS_Wire MakeWire(const TopTools_ListOfShape& edges);
	TopoDS_Face MakeFace(const TopoDS_Wire& outer, const TopoDS_Wire& inner);
	//Build a face and an outer bound and inner wire loops, as profile defs are 2d the xy plane is assumed for the surface
	TopoDS_Face MakeFace(const TopoDS_Wire& wire, const TopTools_SequenceOfShape& innerLoops);
	TopoDS_Wire BuildRectangle(double dimX, double dimY, const TopLoc_Location& location);
	TopoDS_Face BuildRectangleHollowProfileDef(const TopLoc_Location& location, double xDim, double yDim, double wallThickness, double outerFilletRadius, double innerFilletRadius, double precision);
};

