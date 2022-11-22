#include "NEdgeFactory.h"
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeEdge2d.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRepLib.hxx>

std::stringstream NEdgeFactory::GetError(BRepLib_EdgeError edgeError)
{
	std::stringstream strm;
	switch (edgeError)
	{
	case BRepLib_PointProjectionFailed:
		strm << "Edge Maker: Point Projection Failed";
	case BRepLib_ParameterOutOfRange:
		strm << "Edge Maker: Parameter Out Of Range";
	case BRepLib_DifferentPointsOnClosedCurve:
		strm << "Edge Maker: Different Points On Closed Curve";
	case BRepLib_PointWithInfiniteParameter:
		strm << "Edge Maker: Point With Infinite Parameter";
	case BRepLib_DifferentsPointAndParameter:
		strm << "Edge Maker: Differents Point And Parameter";
	case BRepLib_LineThroughIdenticPoints:
		strm << "Edge Maker: Line Through Identical Points";
	default:
		strm << "Edge Maker: Unknown Error";
	}
	return strm;
}


TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom_Curve)  hCurve)
{
	try
	{
		BRepLib_MakeEdge edgeMaker;
		edgeMaker.Init(hCurve);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(GetError(edgeMaker.Error()));
		else
			return edgeMaker.Edge();

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Edge();
}

TopoDS_Edge NEdgeFactory::BuildEdge(Handle(Geom2d_Curve)  hCurve2d)
{
	try
	{
		BRepLib_MakeEdge2d edgeMaker(hCurve2d);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(GetError(edgeMaker.Error()));
		else
		{
			bool ok = BRepLib::BuildCurve3d(edgeMaker.Edge());
			if (!ok)
				Standard_Failure::Raise("Error building 3d curves for edge");
			else
				return edgeMaker.Edge();
		}
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Edge();
}


TopoDS_Edge NEdgeFactory::BuildEdge(const gp_Pnt2d& start, const gp_Pnt2d& end)
{
	try
	{
		BRepLib_MakeEdge2d edgeMaker(start, end);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(GetError(edgeMaker.Error()));
		else
		{
			bool ok = BRepLib::BuildCurve3d(edgeMaker.Edge());
			if (!ok)
				Standard_Failure::Raise("Error building 3d curves for edge");
			else
				return edgeMaker.Edge();
		}
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Edge();
}

TopoDS_Edge NEdgeFactory::BuildEdge(const gp_Pnt& start, const gp_Pnt& end)
{
	try
	{
		BRepLib_MakeEdge edgeMaker(start, end);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(GetError(edgeMaker.Error()));
		else
			return edgeMaker.Edge();
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);

	}
	return TopoDS_Edge();
}