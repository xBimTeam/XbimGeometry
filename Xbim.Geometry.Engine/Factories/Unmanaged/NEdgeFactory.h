#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Edge.hxx>
#include <BRepLib_EdgeError.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <BRep_Builder.hxx>

class NEdgeFactory : public NFactoryBase
{
public:

	static std::stringstream GetError(BRepLib_EdgeError edgeError);
	/// <summary>
	/// Build the edge creating new start and end vertices
	/// </summary>
	/// <param name="hCurve"></param>
	/// <returns></returns>
	 TopoDS_Edge BuildEdge(const Handle(Geom_Curve)& hCurve);
	/// <summary>
	/// Build the edge creating new start and end vertices
	/// </summary>
	/// <param name="hCurve"></param>
	/// <returns></returns>
	 TopoDS_Edge BuildEdge(const Handle(Geom2d_Curve)& hCurve);
	TopoDS_Edge BuildEdge(const gp_Pnt& start, const gp_Pnt& end);
	TopoDS_Edge BuildEdge(const gp_Pnt2d& start, const gp_Pnt2d& end);
	/// builds the edge re-using the start and end vertices and adjusting tolerance to ensure the points intersect the curve within the required gap
	TopoDS_Edge BuildEdge(const Handle(Geom_Curve)& edgeGeom,  TopoDS_Vertex& startVertex,  TopoDS_Vertex& endVertex, double maxTolerance);

	
};

