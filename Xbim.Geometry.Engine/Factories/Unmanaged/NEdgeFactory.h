#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Edge.hxx>
#include <BRepLib_EdgeError.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <BRep_Builder.hxx>

class NEdgeFactory : NFactoryBase
{
public:

	std::stringstream GetError(BRepLib_EdgeError edgeError);

	TopoDS_Edge BuildEdge(Handle(Geom_Curve) hCurve);
	TopoDS_Edge BuildEdge(Handle(Geom2d_Curve) hCurve);
	TopoDS_Edge BuildEdge(const gp_Pnt& start, const gp_Pnt& end);
	TopoDS_Edge BuildEdge(const gp_Pnt2d& start, const gp_Pnt2d& end);
};

