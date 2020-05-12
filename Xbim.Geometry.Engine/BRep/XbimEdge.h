#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Edge.hxx>

public ref class XbimEdge: XbimHandle<TopoDS_Edge>
{
public:
	XbimEdge(const TopoDS_Edge& edge) : XbimHandle(new TopoDS_Edge(edge)) {  };
};

