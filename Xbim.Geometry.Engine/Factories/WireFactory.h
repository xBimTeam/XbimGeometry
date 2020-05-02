#pragma once
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <BRep_Builder.hxx>


class WireFactory
{
protected:
	
public:
	// Builds a wire from a list of contiguous edges, duplicate vertices at nodes are removed
	static int Make(
		int contextId,
		const char* contextLabel, 
		TopTools_ListOfShape& edgeList,
		double millimeter, 
		TopoDS_Wire& resultWire	);	
private:
	
	static void AddEdge(TopoDS_Wire& resultWire, 
		const TopoDS_Edge& edgeToAdd, 
		const TopoDS_Vertex& edgeVertexToJoin, 
		gp_Pnt edgePointToJoin, 
		const TopoDS_Vertex&
		nextEdgeVertex, 
		const TopoDS_Vertex& wireVertexToJoin, 
		gp_Pnt wirePointToJoin, 
		double distance);
};

