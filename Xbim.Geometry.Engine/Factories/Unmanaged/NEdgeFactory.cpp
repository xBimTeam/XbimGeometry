#include "NEdgeFactory.h"
#include <BRepLib_MakeEdge.hxx>
TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom_Curve)  hCurve)
{
	BRepLib_MakeEdge edgeMaker;
	//edgeMaker.Init(hCurve,)
	return TopoDS_Edge();
}

TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom2d_Curve)  hCurve)
{
	return TopoDS_Edge();
}