#include "NEdgeFactory.h"
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeEdge2d.hxx>
TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom_Curve)  hCurve)
{
	BRepLib_MakeEdge edgeMaker;
	//edgeMaker.Init(hCurve,)
	return TopoDS_Edge();
}

TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom2d_Curve)  hCurve2d)
{
	try
	{
		BRepLib_MakeEdge2d edgeMaker(hCurve2d);
		if (!edgeMaker.IsDone())
		{
			BRepLib_EdgeError edgeError = edgeMaker.Error();
			switch (edgeError)
			{			
			case BRepLib_PointProjectionFailed:
				break;
			case BRepLib_ParameterOutOfRange:
				break;
			case BRepLib_DifferentPointsOnClosedCurve:
				break;
			case BRepLib_PointWithInfiniteParameter:
				break;
			case BRepLib_DifferentsPointAndParameter:
				break;
			case BRepLib_LineThroughIdenticPoints:
				break;
			default:
				break;
			}
		}
	}
	catch (Standard_Failure e)
	{
			
	}
	

	return TopoDS_Edge();
}