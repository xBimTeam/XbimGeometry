#include "NEdgeFactory.h"
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeEdge2d.hxx>
#include <Geom_TrimmedCurve.hxx>
TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom_Curve)  hCurve)
{
	try
	{
		BRepLib_MakeEdge edgeMaker;
		edgeMaker.Init(hCurve);
		if (!edgeMaker.IsDone())
		{
			BRepLib_EdgeError edgeError = edgeMaker.Error();
			switch (edgeError)
			{			
			case BRepLib_PointProjectionFailed:
				Standard_Failure::Raise("Edge Maker: Point Projection Failed");
			case BRepLib_ParameterOutOfRange:
				Standard_Failure::Raise("Edge Maker: Parameter Out Of Range");
			case BRepLib_DifferentPointsOnClosedCurve:
				Standard_Failure::Raise("Edge Maker: Different Points On Closed Curve");
			case BRepLib_PointWithInfiniteParameter:
				Standard_Failure::Raise("Edge Maker: Point With Infinite Parameter");
			case BRepLib_DifferentsPointAndParameter:
				Standard_Failure::Raise("Edge Maker: Differents Point And Parameter");
			case BRepLib_LineThroughIdenticPoints:
				Standard_Failure::Raise("Edge Maker: Line Through Identical Points");
			case BRepLib_EdgeDone:
				return edgeMaker.Edge();
			default:
				Standard_Failure::Raise("Edge Maker: Unknown Error");
			}
			return edgeMaker.Edge();
		}
		else
			return edgeMaker.Edge();

	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream strm;
		sf.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	return _emptyEdge;
}

TopoDS_Edge NEdgeFactory::BuildEdge(const gp_Pnt& start, const gp_Pnt& end)
{
	//make the curve then call standard edge builder funtion to handle any errors
	Handle(Geom_TrimmedCurve) tc = _curveFactory.BuildBoundedLine3d(start, end);
	if (!tc.IsNull())
		return BuildEdge(tc);
	else
		return _emptyEdge;
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
				Standard_Failure::Raise("Edge Maker: Point Projection Failed");
			case BRepLib_ParameterOutOfRange:
				Standard_Failure::Raise("Edge Maker: Parameter Out Of Range");
			case BRepLib_DifferentPointsOnClosedCurve:
				Standard_Failure::Raise("Edge Maker: Different Points On Closed Curve");
			case BRepLib_PointWithInfiniteParameter:
				Standard_Failure::Raise("Edge Maker: Point With Infinite Parameter");
			case BRepLib_DifferentsPointAndParameter:
				Standard_Failure::Raise("Edge Maker: Differents Point And Parameter");
			case BRepLib_LineThroughIdenticPoints:
				Standard_Failure::Raise("Edge Maker: Line Through Identical Points");
			case BRepLib_EdgeDone:
				break;
			default:
				Standard_Failure::Raise("Edge Maker: Unknown Error");
			}
			return edgeMaker.Edge();
		}
		else
			Standard_Failure::Raise("Edge Maker: Unknown Error");
	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream strm;
		sf.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	return _emptyEdge;
}