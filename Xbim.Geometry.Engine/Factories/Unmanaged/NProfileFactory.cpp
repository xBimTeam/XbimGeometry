#include "NProfileFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepLib_MakeEdge.hxx>
#include "NEdgeFactory.h"
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepLib.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <gp_Pln.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TopExp_Explorer.hxx>

TopoDS_Compound NProfileFactory::MakeCompound(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
	TopoDS_Compound compound;
	BRep_Builder builder;
	builder.MakeCompound(compound);
	builder.Add(compound, shape1);
	builder.Add(compound, shape2);
	return compound;
}

TopoDS_Face NProfileFactory::MakeFace(const TopoDS_Wire& wire)
{
	try
	{

		BRepBuilderAPI_MakeFace faceMaker(_xyPlane, wire, true);
		if (faceMaker.IsDone())
			return faceMaker.Face();
		else
		{
			BRepBuilderAPI_FaceError error = faceMaker.Error();
			switch (error)
			{
			case BRepBuilderAPI_FaceDone: //should never happen
				pLoggingService->LogInformation("BRepBuilderAPI_FaceDone: ignore");
				break;
			case BRepBuilderAPI_NoFace:
				pLoggingService->LogWarning("BRepBuilderAPI_NoFace");
				break;
			case BRepBuilderAPI_NotPlanar:
				pLoggingService->LogWarning("BRepBuilderAPI_NotPlanar");
				break;
			case BRepBuilderAPI_CurveProjectionFailed:
				pLoggingService->LogWarning("BRepBuilderAPI_CurveProjectionFailed");
				break;
			case BRepBuilderAPI_ParametersOutOfRange:
				pLoggingService->LogWarning("BRepBuilderAPI_ParametersOutOfRange");
				break;
			default:
				break;
			}
		}
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Failed to build face");
	return TopoDS_Face();
}

TopoDS_Edge NProfileFactory::MakeEdge(const gp_Pnt& start, const gp_Pnt& end)
{
	try
	{
		BRepLib_MakeEdge edgeMaker(start, end);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(NEdgeFactory::GetError(edgeMaker.Error()));
		else
			return edgeMaker.Edge();

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Edge();
}

TopoDS_Edge NProfileFactory::MakeEdge(const gp_Pnt2d& start, const gp_Pnt2d& end)
{
	try
	{
		BRepLib_MakeEdge edgeMaker(gp_Pnt(start.X(), start.Y(), 0), gp_Pnt(end.X(), end.Y(), 0));
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(NEdgeFactory::GetError(edgeMaker.Error()));
		else
			return edgeMaker.Edge();

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Edge();
}


TopoDS_Edge NProfileFactory::MakeEdge(const Handle(Geom_Curve)& hCurve)
{
	try
	{
		BRepLib_MakeEdge edgeMaker;
		edgeMaker.Init(hCurve);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(NEdgeFactory::GetError(edgeMaker.Error()));
		else
			return edgeMaker.Edge();

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Edge();
}

TopoDS_Edge NProfileFactory::MakeEdge(const Handle(Geom2d_Curve)& hCurve2d)
{
	try
	{
		BRepLib_MakeEdge2d edgeMaker(hCurve2d);
		if (!edgeMaker.IsDone())
			Standard_Failure::Raise(NEdgeFactory::GetError(edgeMaker.Error()));
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

TopoDS_Wire NProfileFactory::MakeWire(const TopoDS_Edge& edge)
{
	try
	{
		BRep_Builder builder;
		TopoDS_Wire wire;
		builder.MakeWire(wire);
		builder.Add(wire, edge);
		return wire;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
		return TopoDS_Wire();
	}
}

TopoDS_Wire NProfileFactory::MakeWire(const TopTools_ListOfShape& edges)
{
	try
	{
		BRepBuilderAPI_MakeWire wireMaker;
		wireMaker.Add(edges);
		if (wireMaker.IsDone())
			return wireMaker.Wire();
		else
			Standard_Failure::Raise("Failed to build profile as a wire");
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);

	}
	return TopoDS_Wire();
}

TopoDS_Face NProfileFactory::MakeFace(const TopoDS_Wire& outer, const TopoDS_Wire& inner)
{
	TopTools_SequenceOfShape inners;
	inners.Append(inner);
	return MakeFace(outer, inners);
}

TopoDS_Face NProfileFactory::MakeFace(const TopoDS_Wire& wire, const TopTools_SequenceOfShape& innerLoops)
{
	try
	{

		BRepBuilderAPI_MakeFace faceMaker(_xyPlane, wire, true);
		for (auto&& inner : innerLoops)
		{
			faceMaker.Add(TopoDS::Wire(inner));
		}
		if (faceMaker.IsDone())
			return faceMaker.Face();
		else
		{
			BRepBuilderAPI_FaceError error = faceMaker.Error();
			switch (error)
			{
			case BRepBuilderAPI_FaceDone: //should never happen
				pLoggingService->LogInformation("BRepBuilderAPI_FaceDone: ignore");
				break;
			case BRepBuilderAPI_NoFace:
				pLoggingService->LogWarning("BRepBuilderAPI_NoFace");
				break;
			case BRepBuilderAPI_NotPlanar:
				pLoggingService->LogWarning("BRepBuilderAPI_NotPlanar");
				break;
			case BRepBuilderAPI_CurveProjectionFailed:
				pLoggingService->LogWarning("BRepBuilderAPI_CurveProjectionFailed");
				break;
			case BRepBuilderAPI_ParametersOutOfRange:
				pLoggingService->LogWarning("BRepBuilderAPI_ParametersOutOfRange");
				break;
			default:
				break;
			}
		}
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Failed to build face");
	return TopoDS_Face();
}

TopoDS_Wire NProfileFactory::BuildRectangle(double dimX, double dimY, const TopLoc_Location& location)
{
	try
	{
		double xOff = dimX / 2;
		double yOff = dimY / 2;
		gp_Pnt bl(-xOff, -yOff, 0);
		gp_Pnt br(xOff, -yOff, 0);
		gp_Pnt tr(xOff, yOff, 0);
		gp_Pnt tl(-xOff, yOff, 0);
		Handle(Geom_TrimmedCurve) aSeg1 = GC_MakeSegment(bl, br);
		Handle(Geom_TrimmedCurve) aSeg2 = GC_MakeSegment(br, tr);
		Handle(Geom_TrimmedCurve) aSeg3 = GC_MakeSegment(tr, tl);
		Handle(Geom_TrimmedCurve) aSeg4 = GC_MakeSegment(tl, bl);
		TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(aSeg1);
		TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(aSeg2);
		TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(aSeg3);
		TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(aSeg4);
		TopoDS_Wire wire = BRepBuilderAPI_MakeWire(e1, e2, e3, e4);
		wire.Closed(true);
		wire.Checked(true);
		//apply the position transformation
		if (!location.IsIdentity()) wire.Move(location);
		return wire;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}

TopoDS_Face NProfileFactory::BuildRectangleHollowProfileDef(const TopLoc_Location& location, double xDim, double yDim, double wallThickness, double outerFilletRadius, double innerFilletRadius, double precision)
{
	try
	{
		double xOff = xDim / 2;
		double yOff = yDim / 2;
		gp_Pnt bl(-xOff, -yOff, 0);
		gp_Pnt br(xOff, -yOff, 0);
		gp_Pnt tr(xOff, yOff, 0);
		gp_Pnt tl(-xOff, yOff, 0);
		//make the vertices
		BRep_Builder builder;
		TopoDS_Vertex vbl, vbr, vtr, vtl;
		builder.MakeVertex(vbl, bl, precision);
		builder.MakeVertex(vbr, br, precision);
		builder.MakeVertex(vtr, tr, precision);
		builder.MakeVertex(vtl, tl, precision);
		//make the edges
		TopoDS_Wire wire;
		builder.MakeWire(wire);
		builder.Add(wire, BRepBuilderAPI_MakeEdge(vbl, vbr));
		builder.Add(wire, BRepBuilderAPI_MakeEdge(vbr, vtr));
		builder.Add(wire, BRepBuilderAPI_MakeEdge(vtr, vtl));
		builder.Add(wire, BRepBuilderAPI_MakeEdge(vtl, vbl));
		wire.Closed(Standard_True);

		if (outerFilletRadius > 0) //consider fillets
		{
			BRepBuilderAPI_MakeFace outerFaceMaker(gp_Pln(), wire, true);
			BRepFilletAPI_MakeFillet2d filleter(outerFaceMaker.Face());
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				filleter.AddFillet(exp.CurrentVertex(), outerFilletRadius);
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More();) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		//make the face
		BRepBuilderAPI_MakeFace faceMaker(gp_Pln(), wire, true);
		TopoDS_Wire innerWire;
		builder.MakeWire(innerWire);
		double t = wallThickness;
		gp_Pnt ibl(-xOff + t, -yOff + t, 0);
		gp_Pnt ibr(xOff - t, -yOff + t, 0);
		gp_Pnt itr(xOff - t, yOff - t, 0);
		gp_Pnt itl(-xOff + t, yOff - t, 0);
		TopoDS_Vertex vibl, vibr, vitr, vitl;
		builder.MakeVertex(vibl, ibl, precision);
		builder.MakeVertex(vibr, ibr, precision);
		builder.MakeVertex(vitr, itr, precision);
		builder.MakeVertex(vitl, itl, precision);
		builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vibl, vibr));
		builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vibr, vitr));
		builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vitr, vitl));
		builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vitl, vibl));


		if (innerFilletRadius > 0) //consider fillets
		{
			BRepBuilderAPI_MakeFace innerFaceMaker(gp_Pln(), innerWire, true);
			BRepFilletAPI_MakeFillet2d filleter(innerFaceMaker.Face());
			for (BRepTools_WireExplorer exp(innerWire); exp.More(); exp.Next())
			{
				filleter.AddFillet(exp.CurrentVertex(), innerFilletRadius);
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More();) //just take the first wire
				{
					innerWire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		innerWire.Reverse();
		innerWire.Closed(Standard_True);
		faceMaker.Add(innerWire);
		if(!faceMaker.IsDone())
			Standard_Failure::Raise("Failed to build profile as a face");
		auto face = faceMaker.Face();
		//apply the position transformation
		if(!location.IsIdentity()) 
			face.Move(location);
		return face;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Face();
}
