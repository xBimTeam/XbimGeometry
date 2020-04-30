#pragma once
#include <TopTools_ListOfShape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
class WireFactory
{
public:
	// Builds a wire from a list of contiguous edges, duplicate vertices at nodes are removed
	static int Make(TopTools_ListOfShape& edgeList, TopoDS_Wire& resultWire);
};

