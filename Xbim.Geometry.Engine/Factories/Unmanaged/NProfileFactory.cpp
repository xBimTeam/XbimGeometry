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

#include <ShapeFix_Face.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <ShapeFix_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>

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
				pLoggingService->LogWarning("BRepBuilderAPI_FaceDone: ignore");
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

/// <summary>
/// This function checks if a wre is suitable to a valid profile area for extrusion as a solid, checks if the wire is clockwise or counterclockwise
/// </summary>
/// <param name="wire"></param>
/// <returns></returns>
TopoDS_Wire NProfileFactory::MakeValidAreaProfileWire(const TopoDS_Wire& wire, bool& isCounterClockwise)
{
	auto tmpFace = MakeFace(wire);
	//we do not attempt to fix every wire, the main problem is typically self intersecting wires or wires with no area
	//self intersection does not occur with 3 edges and a positive area
	if (wire.NbChildren() < 4)
	{
		double thisArea = getArea(tmpFace);
		if (std::abs(thisArea) < Precision::Approximation())
			return TopoDS_Wire();
		else
		{
			isCounterClockwise = thisArea > 0;
			return wire;
		}
	}
	
	ShapeFix_Shape faceFixer(tmpFace);
	
	if (faceFixer.Perform())
	{
		if (faceFixer.Shape().ShapeType() == TopAbs_COMPOUND || faceFixer.Shape().ShapeType() == TopAbs_SHELL)
		{
			TopoDS_Face fixed;
			//take the wire with the larget area
			double area = 0;
			for (TopExp_Explorer shellExp(faceFixer.Shape(), TopAbs_FACE); shellExp.More(); shellExp.Next())
			{
				auto&& currentFace = TopoDS::Face(shellExp.Current());		
				double thisArea = getArea(currentFace);
				double absArea = std::abs(thisArea);
				if(absArea > area)
				{
					area = thisArea;
					fixed = currentFace;
					isCounterClockwise = thisArea > 0;
				}
			}	
			if (area < Precision::Approximation()) 
				return TopoDS_Wire();
			else
				return BRepTools::OuterWire(fixed);
		}
		else if (faceFixer.Shape().ShapeType() == TopAbs_FACE)
		{
			tmpFace = TopoDS::Face(faceFixer.Shape());
			double thisArea = getArea(tmpFace);
			if (std::abs(thisArea) < Precision::Approximation()) 
				return TopoDS_Wire();
			else
			{
				isCounterClockwise = thisArea > 0;
				return BRepTools::OuterWire( tmpFace); //in this test we only expect an outer, holes are not valid and ignored
			}
		}
			 
		else
			Standard_Failure::Raise("Error fixing profile wire");
	}
	double thisArea = getArea(tmpFace);
	if (std::abs(thisArea) < Precision::Approximation()) 
		return TopoDS_Wire();
	isCounterClockwise = thisArea > 0;
	return wire;
}

/// <summary>
/// Calculates the area of the wire, assumes only one wire on the face, for use on in this factory, a negative area value indicates clockwise winding
/// </summary>
/// <returns></returns>
double NProfileFactory::getArea(const TopoDS_Face& face)
{
	GProp_GProps gProps;
	BRepGProp::SurfaceProperties(face, gProps);
	return gProps.Mass();
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
TopoDS_Face NProfileFactory::MakeFace(const TopoDS_Wire& outer, const TopoDS_Wire& inner, double tolerance)
{
	TopTools_SequenceOfShape inners;
	inners.Append(inner);
	return MakeFace(outer, inners, tolerance);
}
//This function will check the orientation of the inner loops to ensure they are reverse of the outer bound
TopoDS_Face NProfileFactory::MakeFace(const TopoDS_Wire& wire, const TopTools_SequenceOfShape& innerLoops, double tolerance)
{
	try
	{

		BRepBuilderAPI_MakeFace faceMaker(_xyPlane, wire, true);
		for (auto&& inner : innerLoops)
		{
			faceMaker.Add(TopoDS::Wire(inner));
		}
		if (faceMaker.IsDone())
		{
			ShapeFix_Face fixFace(faceMaker.Face());
			fixFace.FixOrientation();// it is necessary to fix the orientation as models vary in the winding consistency of loops, often the inner loops are not CCW or opposite to the outer loops 
			return fixFace.Face();
		}
		else
		{
			BRepBuilderAPI_FaceError error = faceMaker.Error();
			switch (error)
			{
			case BRepBuilderAPI_FaceDone: //should never happen
				pLoggingService->LogWarning("BRepBuilderAPI_FaceDone: ignore");
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
		if (!faceMaker.IsDone())
			Standard_Failure::Raise("Failed to build profile as a face");
		auto face = faceMaker.Face();
		//apply the position transformation
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		return face;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Face();
}
TopoDS_Wire NProfileFactory::BuildRoundedRectangle(double dimX, double dimY, double roundingRadius, const TopLoc_Location& location, double precision)
{
	try
	{
		//make the basic shapes
		double xOff = dimX / 2;
		double yOff = dimY / 2;
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

		if (roundingRadius > 0) //consider fillets
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				filleter.AddFillet(exp.CurrentVertex(), roundingRadius);
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}

		//apply the position transformation
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		return wire;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}

TopoDS_Wire NProfileFactory::BuildAsymmetricIShape(double bottomFlangeWidth, double overallDepth, double webThickness, double bottomFlangeThickness,
	double bottomFlangeFilletRadius, double topFlangeWidth, double topFlangeThickness, double topFlangeFilletRadius,
	double bottomFlangeEdgeRadius, double bottomFlangeSlope, double topFlangeEdgeRadius, double topFlangeSlope,
	const TopLoc_Location& location, double precision, bool detailed)
{
	try
	{
		double dXTop = topFlangeWidth / 2;
		double dXBottom = bottomFlangeWidth / 2;
		double dY = overallDepth / 2;
		double tFTop = topFlangeThickness;
		double tFBottom = bottomFlangeThickness;
		double tW = webThickness;

		gp_Pnt p1(-dXTop, dY, 0);
		gp_Pnt p2(dXTop, dY, 0);
		gp_Pnt p3(dXTop, dY - tFTop, 0);
		gp_Pnt p4(tW / 2, dY - tFTop, 0);
		gp_Pnt p5(tW / 2, -dY + tFBottom, 0);
		gp_Pnt p6(dXBottom, -dY + tFBottom, 0);
		gp_Pnt p7(dXBottom, -dY, 0);
		gp_Pnt p8(-dXBottom, -dY, 0);
		gp_Pnt p9(-dXBottom, -dY + tFBottom, 0);
		gp_Pnt p10(-tW / 2, -dY + tFBottom, 0);
		gp_Pnt p11(-tW / 2, dY - tFTop, 0);
		gp_Pnt p12(-dXTop, dY - tFTop, 0);

		TopoDS_Vertex v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12;
		BRep_Builder b;
		double t = precision;
		b.MakeVertex(v1, p1, t);
		b.MakeVertex(v2, p2, t);
		b.MakeVertex(v3, p3, t);
		b.MakeVertex(v4, p4, t);
		b.MakeVertex(v5, p5, t);
		b.MakeVertex(v6, p6, t);
		b.MakeVertex(v7, p7, t);
		b.MakeVertex(v8, p8, t);
		b.MakeVertex(v9, p9, t);
		b.MakeVertex(v10, p10, t);
		b.MakeVertex(v11, p11, t);
		b.MakeVertex(v12, p12, t);

		BRepBuilderAPI_MakePolygon polyMaker;
		polyMaker.Add(v1);
		polyMaker.Add(v2);
		polyMaker.Add(v3);
		polyMaker.Add(v4);
		polyMaker.Add(v5);
		polyMaker.Add(v6);
		polyMaker.Add(v7);
		polyMaker.Add(v8);
		polyMaker.Add(v9);
		polyMaker.Add(v10);
		polyMaker.Add(v11);
		polyMaker.Add(v12);
		polyMaker.Close();
		TopoDS_Wire wire = polyMaker.Wire();
		if (!polyMaker.IsDone())
			Standard_Failure::Raise("Failed to build asymetric I Shaped profile");
		//need to consider higher detail with sloping flange and end fillet radius
		if (detailed && (!double::IsNaN(topFlangeFilletRadius) || !double::IsNaN(bottomFlangeFilletRadius)))
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());

			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if (!double::IsNaN(topFlangeFilletRadius) && (i == 4 || i == 5))
					filleter.AddFillet(exp.CurrentVertex(), topFlangeFilletRadius);
				if (!double::IsNaN(bottomFlangeFilletRadius) && (i == 10 || i == 11))
					filleter.AddFillet(exp.CurrentVertex(), bottomFlangeFilletRadius);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		return wire;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Wire NProfileFactory::BuildCShape(double width, double depth, double girth, double wallThickness, double internalFilletRadius, const TopLoc_Location& location, bool detailed)
{
	try
	{

		double dX = width / 2;
		double dY = depth / 2;
		double dG = girth;
		double tW = wallThickness;

		BRepBuilderAPI_MakeWire wireMaker;
		if (dG > 0)	// has Girth
		{
			if (fabs(tW - dG) < Precision::Confusion())
			{
				// Girth < wall thickness
				gp_Pnt p1(-dX, dY, 0);
				gp_Pnt p2(dX, dY, 0);
				gp_Pnt p3(dX, dY - dG, 0);
				gp_Pnt p4(dX - tW, dY - dG, 0);
				gp_Pnt p6(-dX + tW, dY - tW, 0);
				gp_Pnt p7(-dX + tW, -dY + tW, 0);
				gp_Pnt p9(dX - tW, -dY + dG, 0);
				gp_Pnt p10(dX, -dY + dG, 0);
				gp_Pnt p11(dX, -dY, 0);
				gp_Pnt p12(-dX, -dY, 0);

				wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p6));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p9));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
			}
			else
			{
				gp_Pnt p1(-dX, dY, 0);
				gp_Pnt p2(dX, dY, 0);
				gp_Pnt p3(dX, dY - dG, 0);
				gp_Pnt p4(dX - tW, dY - dG, 0);
				gp_Pnt p5(dX - tW, dY - tW, 0);
				gp_Pnt p6(-dX + tW, dY - tW, 0);
				gp_Pnt p7(-dX + tW, -dY + tW, 0);
				gp_Pnt p8(dX - tW, -dY + tW, 0);
				gp_Pnt p9(dX - tW, -dY + dG, 0);
				gp_Pnt p10(dX, -dY + dG, 0);
				gp_Pnt p11(dX, -dY, 0);
				gp_Pnt p12(-dX, -dY, 0);

				wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
			}
		}
		else
		{
			gp_Pnt p1(-dX, dY, 0);
			gp_Pnt p2(dX, dY, 0);
			gp_Pnt p5(dX, dY - tW, 0);
			gp_Pnt p6(-dX + tW, dY - tW, 0);
			gp_Pnt p7(-dX + tW, -dY + tW, 0);
			gp_Pnt p8(dX, -dY + tW, 0);
			gp_Pnt p11(dX, -dY, 0);
			gp_Pnt p12(-dX, -dY, 0);

			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p11));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
		}

		TopoDS_Wire wire = wireMaker.Wire();

		if (detailed && !double::IsNaN(internalFilletRadius)) //consider fillets
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
			double iRad = internalFilletRadius;
			double oRad = iRad + tW;
			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if (i == 1 || i == 2 || i == 11 || i == 12)
					filleter.AddFillet(exp.CurrentVertex(), oRad);
				else if (i == 5 || i == 6 || i == 7 || i == 8)
					filleter.AddFillet(exp.CurrentVertex(), iRad);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		return wire;

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Wire NProfileFactory::BuildIShape(double overallWidth, double overallDepth, double flangeThickness, double webThickness, double filletRadius, const TopLoc_Location& location, double precision, bool detailed)
{
	try
	{
		double dX = overallWidth / 2;
		double dY = overallDepth / 2;
		double tF = flangeThickness;
		double tW = webThickness;

		gp_Pnt p1(-dX, dY, 0);
		gp_Pnt p2(dX, dY, 0);
		gp_Pnt p3(dX, dY - tF, 0);
		gp_Pnt p4(tW / 2, dY - tF, 0);
		gp_Pnt p5(tW / 2, -dY + tF, 0);
		gp_Pnt p6(dX, -dY + tF, 0);
		gp_Pnt p7(dX, -dY, 0);
		gp_Pnt p8(-dX, -dY, 0);
		gp_Pnt p9(-dX, -dY + tF, 0);
		gp_Pnt p10(-tW / 2, -dY + tF, 0);
		gp_Pnt p11(-tW / 2, dY - tF, 0);
		gp_Pnt p12(-dX, dY - tF, 0);

		TopoDS_Vertex v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12;
		BRep_Builder b;
		double t = precision;
		b.MakeVertex(v1, p1, t);
		b.MakeVertex(v2, p2, t);
		b.MakeVertex(v3, p3, t);
		b.MakeVertex(v4, p4, t);
		b.MakeVertex(v5, p5, t);
		b.MakeVertex(v6, p6, t);
		b.MakeVertex(v7, p7, t);
		b.MakeVertex(v8, p8, t);
		b.MakeVertex(v9, p9, t);
		b.MakeVertex(v10, p10, t);
		b.MakeVertex(v11, p11, t);
		b.MakeVertex(v12, p12, t);

		BRepBuilderAPI_MakePolygon polyMaker;
		polyMaker.Add(v1);
		polyMaker.Add(v2);
		polyMaker.Add(v3);
		polyMaker.Add(v4);
		polyMaker.Add(v5);
		polyMaker.Add(v6);
		polyMaker.Add(v7);
		polyMaker.Add(v8);
		polyMaker.Add(v9);
		polyMaker.Add(v10);
		polyMaker.Add(v11);
		polyMaker.Add(v12);
		polyMaker.Close();
		TopoDS_Wire wire = polyMaker.Wire();
		if (!polyMaker.IsDone())
			Standard_Failure::Raise("Failed to build I Shaped profile");
		//need to consider higher detail with soping flange and end fillet radius
		if (detailed && filletRadius > 0)
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());

			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if (i == 4 || i == 5 || i == 10 || i == 11)
					filleter.AddFillet(exp.CurrentVertex(), filletRadius);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		return wire;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Wire NProfileFactory::BuildLShape(double depth, double width, double thickness, double legSlope, double edgeRadius, double filletRadius, const TopLoc_Location& location, double angleToRadians, bool detailed)
{
	try
	{
		double dY = depth / 2;
		double dX;
		if (!double::IsNaN(width))
			dX = width / 2;
		else
			dX = dY;
		double tF = thickness;
		gp_Pnt p1(-dX, dY, 0);
		gp_Pnt p2(-dX + tF, dY, 0);
		gp_Pnt p3(-dX + tF, -dY + tF, 0);
		if (detailed && !double::IsNaN(legSlope))
		{

			p3.SetX(p3.X() + (((dY * 2) - tF) * System::Math::Tan(legSlope * angleToRadians)));
			p3.SetY(p3.Y() + (((dX * 2) - tF) * System::Math::Tan(legSlope * angleToRadians)));
		}
		gp_Pnt p4(dX, -dY + tF, 0);
		gp_Pnt p5(dX, -dY, 0);
		gp_Pnt p6(-dX, -dY, 0);

		BRepBuilderAPI_MakeWire wireMaker;
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p1));
		TopoDS_Wire wire = wireMaker.Wire();
		wire.Closed(Standard_True);
		if (detailed && (!double::IsNaN(edgeRadius) || !double::IsNaN(filletRadius))) //consider fillets
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());

			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if ((i == 2 || i == 4) && !double::IsNaN(edgeRadius))
					filleter.AddFillet(exp.CurrentVertex(), edgeRadius);
				else if (i == 3 && !double::IsNaN(filletRadius))
					filleter.AddFillet(exp.CurrentVertex(), filletRadius);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		
		//removed not in Ifc4
		/*if (profile->CentreOfGravityInX.HasValue || profile->CentreOfGravityInY.HasValue)
		{
			double transX = 0;
			double transY = 0;
			if (profile->CentreOfGravityInX.HasValue) transX = profile->CentreOfGravityInX.Value;
			if (profile->CentreOfGravityInY.HasValue) transY = profile->CentreOfGravityInY.Value;
			gp_Vec v(transX, transY, 0);
			gp_Trsf t;
			t.SetTranslation(v);
			wire.Move(t);
		}*/
		return wire;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Wire NProfileFactory::BuildTShape(double flangeWidth, double depth, double flangeThickness, double webThickness,
	double flangeSlope, double webSlope, double flangeEdgeRadius, double filletRadius, double webEdgeRadius,
	const TopLoc_Location& location, double angleToRadians, bool detailed)
{
	try
	{

		double dX = flangeWidth / 2;
		double dY = depth / 2;
		double tF = flangeThickness;
		double tW = webThickness;

		gp_Pnt p1(-dX, dY, 0);
		gp_Pnt p2(dX, dY, 0);
		gp_Pnt p3(dX, dY - tF, 0);
		gp_Pnt p4(tW / 2, dY - tF, 0);
		gp_Pnt p5(tW / 2, -dY, 0);
		gp_Pnt p6(-tW / 2, -dY, 0);
		gp_Pnt p7(-tW / 2, dY - tF, 0);
		gp_Pnt p8(-dX, dY - tF, 0);

		if (detailed && (!double::IsNaN(flangeSlope) || !double::IsNaN(webSlope)))
		{
			double fSlope = 0;
			if (!double::IsNaN(flangeSlope)) fSlope = flangeSlope;
			double wSlope = 0;
			if (!double::IsNaN(webSlope)) wSlope = webSlope;
			double bDiv4 = flangeWidth / 4;

			p3.SetY(p3.Y() + (bDiv4 * System::Math::Tan(fSlope * angleToRadians)));
			p8.SetY(p8.Y() + (bDiv4 * System::Math::Tan(fSlope * angleToRadians)));


			gp_Lin2d flangeLine(gp_Pnt2d(bDiv4, dY - tF), gp_Dir2d(1, System::Math::Tan(fSlope * angleToRadians)));
			gp_Lin2d webLine(gp_Pnt2d(tW / 2.0, 0), gp_Dir2d(System::Math::Tan(wSlope * angleToRadians), 1));
			IntAna2d_AnaIntersection intersector(flangeLine, webLine);
			const IntAna2d_IntPoint& intersectPoint = intersector.Point(1);
			gp_Pnt2d p2d = intersectPoint.Value();

			p4.SetX(p2d.X());
			p4.SetY(p2d.Y());
			p7.SetX(-p2d.X());
			p7.SetY(p2d.Y());

			p5.SetX(p5.X() - (dY * System::Math::Tan(wSlope * angleToRadians)));
			p6.SetX(p6.X() + (dY * System::Math::Tan(wSlope * angleToRadians)));
		}

		BRepBuilderAPI_MakeWire wireMaker;

		wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1));
		TopoDS_Wire wire = wireMaker.Wire();
		if (detailed && (!double::IsNaN(flangeEdgeRadius) || !double::IsNaN(filletRadius) || !double::IsNaN(webEdgeRadius))) //consider fillets
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if ((i == 3 || i == 8) && !double::IsNaN(flangeEdgeRadius))
					filleter.AddFillet(exp.CurrentVertex(), flangeEdgeRadius);
				else if ((i == 4 || i == 7) && !double::IsNaN(filletRadius))
					filleter.AddFillet(exp.CurrentVertex(), filletRadius);
				else if ((i == 5 || i == 6) && !double::IsNaN(webEdgeRadius))
					filleter.AddFillet(exp.CurrentVertex(), webEdgeRadius);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		//removed in Ifc4
		/*if (profile->CentreOfGravityInY.HasValue)
		{
			gp_Vec v( 0, profile->CentreOfGravityInY.Value, 0);
			gp_Trsf t;
			t.SetTranslation(v);
			wire.Move(t);
		}*/
		return wire;

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Wire NProfileFactory::BuildTrapezium(double bottomDimX, double topDimX, double dimY,double topOffsetX, const TopLoc_Location& location)
{
	try
	{
		double xOffTopLeft = -((bottomDimX / 2) + topOffsetX);
		double xOffTopRight = topDimX;
		double xOffBottomLeft = -(bottomDimX / 2);
		double xOffBottomRight = (bottomDimX / 2);
		double yOff = dimY / 2;
		gp_Pnt bl(xOffBottomLeft, -yOff, 0);
		gp_Pnt br(xOffBottomRight, -yOff, 0);
		gp_Pnt tr(xOffTopRight, yOff, 0);
		gp_Pnt tl(xOffTopLeft, yOff, 0);
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
TopoDS_Wire NProfileFactory::BuildUShape(double flangeWidth, double depth, double flangeThickness, double webThickness, double flangeSlope, double edgeRadius, double filletRadius, const TopLoc_Location& location, double radianFactor, bool detailed)
{
	try
	{
		double dX = flangeWidth / 2;
		double dY = depth / 2;
		double tF = flangeThickness;
		double tW = webThickness;

		gp_Pnt p1(-dX, dY, 0);
		gp_Pnt p2(dX, dY, 0);
		gp_Pnt p3(dX, dY - tF, 0);
		gp_Pnt p4(-dX + tW, dY - tF, 0);

		gp_Pnt p5(-dX + tW, -dY + tF, 0);
		gp_Pnt p6(dX, -dY + tF, 0);
		gp_Pnt p7(dX, -dY, 0);
		gp_Pnt p8(-dX, -dY, 0);

		if (detailed && !double::IsNaN(flangeSlope))
		{
			p4.SetY(p4.Y() - (((dX * 2) - tW) * System::Math::Tan(flangeSlope * radianFactor)));
			p5.SetY(p5.Y() + (((dX * 2) - tW) * System::Math::Tan(flangeSlope * radianFactor)));
		}

		BRepBuilderAPI_MakeWire wireMaker;
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1));
		TopoDS_Wire wire = wireMaker.Wire();
		wire.Closed(Standard_True);

		if (detailed && (!double::IsNaN(edgeRadius) || !double::IsNaN(filletRadius))) //consider fillets
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());

			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if ((i == 3 || i == 6) && !double::IsNaN(edgeRadius))
					filleter.AddFillet(exp.CurrentVertex(), edgeRadius);
				else if ((i == 4 || i == 5) && !double::IsNaN(filletRadius))
					filleter.AddFillet(exp.CurrentVertex(), filletRadius);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		//removed in Ifc4
		/*if (profile->CentreOfGravityInX.HasValue)
		{
			gp_Vec v(profile->CentreOfGravityInX.Value, 0, 0);
			gp_Trsf t;
			t.SetTranslation(v);
			wire.Move(t);
		}*/
		return wire;

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Wire NProfileFactory::BuildZShape(double flangeWidth, double depth, double flangeThickness, double webThickness, double edgeRadius, double filletRadius, const TopLoc_Location& location, bool detailed)
{
	try
	{

		double dX = flangeWidth;
		double dY = depth / 2;
		double tF = flangeThickness;
		double tW = webThickness;


		gp_Pnt p1(-dX + (tW / 2), dY, 0);
		gp_Pnt p2(tW / 2, dY, 0);
		gp_Pnt p3(tW / 2, -dY + tF, 0);
		gp_Pnt p4(dX - tW / 2, -dY + tF, 0);
		gp_Pnt p5(dX - tW / 2, -dY, 0);
		gp_Pnt p6(-tW / 2, -dY, 0);
		gp_Pnt p7(-tW / 2, dY - tF, 0);
		gp_Pnt p8(-dX + (tW / 2), dY - tF, 0);


		BRepBuilderAPI_MakeWire wireMaker;

		wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
		wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1));

		TopoDS_Wire wire = wireMaker.Wire();

		if (detailed && (!double::IsNaN(filletRadius) || !double::IsNaN(edgeRadius)))
		{
			BRepBuilderAPI_MakeFace faceMaker(wire, true);
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
			int i = 1;
			for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
			{
				if ((i == 3 || i == 7) && !double::IsNaN(filletRadius))
					filleter.AddFillet(exp.CurrentVertex(), filletRadius);
				else if ((i == 4 || i == 8) && !double::IsNaN(edgeRadius))
					filleter.AddFillet(exp.CurrentVertex(), edgeRadius);
				i++;
			}
			filleter.Build();
			if (filleter.IsDone())
			{
				TopoDS_Shape shape = filleter.Shape();
				for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
				{
					wire = TopoDS::Wire(exp.Current());
					break;
				}
			}
		}
		wire.Closed(true);
		if (!location.IsIdentity())
			wire.Move(location);
		return wire;

	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Wire();
}
TopoDS_Face NProfileFactory::BuildMirrored(const TopoDS_Face& parentFace)
{
	try
	{
		//we need to mirror about the Y axis
		gp_Pnt origin(0, 0, 0);
		gp_Dir xDir(0, 1, 0);
		gp_Ax1 mirrorAxis(origin, xDir);
		gp_Trsf aTrsf;
		aTrsf.SetMirror(mirrorAxis);
		BRepBuilderAPI_Transform aBrepTrsf(parentFace, aTrsf);
		if (!aBrepTrsf.IsDone())
			Standard_Failure::Raise("Could not mirror profile face");
		return TopoDS::Face(aBrepTrsf.Shape().Reversed());
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
	}
	return TopoDS_Face();
}


