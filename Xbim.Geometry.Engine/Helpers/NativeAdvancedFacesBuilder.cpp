#include "NativeAdvancedFacesBuilder.h"
#include "../Helpers/XbimNativeApi.h"


TopoDS_Shape NativeAdvancedFacesBuilder::BuildShell(BuildShellParams params)
{
	TopoDS_Shell shell;
	builder.MakeShell(shell);

	try
	{
		for (auto& faceData : params.facesParams)
		{
			TopoDS_Face faceShape = BuildFace(faceData, builder);
			if (!faceShape.IsNull())
			{
				builder.Add(shell, faceShape);
			}
		}

		BRepCheck_Shell checker(shell);
		BRepCheck_Status st = checker.Orientation();
		if (st != BRepCheck_Status::BRepCheck_NoError)
		{
			std::string errMsg;
			if (!XbimNativeApi::FixShell(shell, 10, errMsg))
			{
				std::string msg = "Failed to fix shell in advanced brep: " + errMsg;
				LogWarning(msg.c_str());
			}
			else
				checker.Init(shell);


			if (checker.Orientation() == BRepCheck_Status::BRepCheck_NoError) //we have succededed
			{
				shell.Closed(true);
				shell.Checked(true);
				return shell;
			}
			else
			{
				//we need to fix the shape and return a compound or a shell
				TopoDS_Shape shape = shell;
				if (!XbimNativeApi::FixShape(shape, 10, errMsg))
				{
					std::string msg = "InitAdvancedFaces: Failed to fix shape: " + errMsg;
					LogWarning(msg.c_str());
				}
				return shape;
			}
		}
		else //it is oriented correctly and closed
		{
			shell.Closed(true);
			shell.Checked(true);
			return shell;
		}
	}
	catch (const Standard_Failure& exc)
	{
		std::stringstream ss;
		ss << "General failure building advanced faces: ";
		ss << exc.GetMessageString();
		LogError(ss.str().c_str());
		return shell;
	}
}


TopoDS_Edge NativeAdvancedFacesBuilder::BuildOrientEdge(BuildEdgeParams p, ShapeFix_Edge& edgeFixer)
{
	if (edgeCurves.IsBound(p.edgeLabel))
	{
		TopoDS_Edge existingEdge = TopoDS::Edge(edgeCurves.Find(p.edgeLabel));
		if (!p.orientation) {
			return TopoDS::Edge(existingEdge.Reversed());
		}
		else {
			return existingEdge;
		}
	}

	TopoDS_Edge topoEdgeCurve;

	// If the curve is closed and start/end vertices are the same,
	// then we can build the entire closed edge from first to last param
	if (p.sharedEdgeGeom->IsClosed() && p.start.IsSame(p.end))
	{
		double f = p.sharedEdgeGeom->FirstParameter();
		double l = p.sharedEdgeGeom->LastParameter();
		topoEdgeCurve = BRepBuilderAPI_MakeEdge(p.sharedEdgeGeom, p.start, p.end, f, l);
	}
	else
	{
		double trimParam1;
		double trimParam2;
		double trim1Tolerance;
		double trim2Tolerance;

		bool foundP1 = LocatePointOnCurve(p.sharedEdgeGeom, p.start, p.sewingTolerance * 20, trimParam1, trim1Tolerance);
		bool foundP2 = LocatePointOnCurve(p.sharedEdgeGeom, p.end, p.sewingTolerance * 20, trimParam2, trim2Tolerance);
		if (!foundP1) //assume before the start of the curve
		{
			std::string msg = "Failed to project vertex to edge geometry: #" + std::to_string(p.edgeLabel) + ", start point assumed";
			LogWarning(msg.c_str());
			trimParam1 = p.sharedEdgeGeom->FirstParameter();
			trim1Tolerance = p.sewingTolerance;
		}
		if (!foundP2) //assume before the start of the curve
		{
			std::string msg = "Failed to project vertex to edge geometry: #" + std::to_string(p.edgeLabel) + ", end point assumed";
			LogWarning(msg.c_str());
			trimParam2 = p.sharedEdgeGeom->LastParameter();
			trim2Tolerance = p.sewingTolerance;
		}

		double currentStartTol = BRep_Tool::Tolerance(p.start);
		double currentEndTol = BRep_Tool::Tolerance(p.end);

		if (trim1Tolerance > currentStartTol)
			builder.UpdateVertex(p.start, trim1Tolerance);
		if (trim2Tolerance > currentEndTol)
			builder.UpdateVertex(p.end, trim2Tolerance);

		// Make the edge:
		BRepBuilderAPI_MakeEdge edgeMaker(p.sharedEdgeGeom, p.start, p.end, trimParam1, trimParam2);
		if (!edgeMaker.IsDone())
		{
			BRepBuilderAPI_EdgeError err = edgeMaker.Error();
			std::stringstream ss;
			ss << "BuildOrientEdge: Failed to create edge #" << p.edgeLabel << ": ";

			switch (err)
			{
				case BRepBuilderAPI_PointProjectionFailed:
					ss << "BRepBuilderAPI_PointProjectionFailed";
					break;
				case BRepBuilderAPI_ParameterOutOfRange:
					ss << "BRepBuilderAPI_ParameterOutOfRange";
					break;
				case BRepBuilderAPI_DifferentPointsOnClosedCurve:
					ss << "BRepBuilderAPI_DifferentPointsOnClosedCurve";
					break;
				case BRepBuilderAPI_PointWithInfiniteParameter:
					ss << "BRepBuilderAPI_PointWithInfiniteParameter";
					break;
				case BRepBuilderAPI_DifferentsPointAndParameter:
					ss << "BRepBuilderAPI_DifferentsPointAndParameter";
					break;
				case BRepBuilderAPI_LineThroughIdenticPoints:
					ss << "BRepBuilderAPI_LineThroughIdenticPoints";
					break;
				default:
					ss << "Unknown error";
					break;
			}
			LogError(ss.str().c_str());
			return TopoDS_Edge();
		}

		topoEdgeCurve = edgeMaker.Edge();
		edgeFixer.FixVertexTolerance(topoEdgeCurve);
	}

	edgeCurves.Bind(p.edgeLabel, topoEdgeCurve);

	if (!p.orientation) {
		topoEdgeCurve = TopoDS::Edge(topoEdgeCurve.Reversed());
	}

	return topoEdgeCurve;
}

void NativeAdvancedFacesBuilder::BuildLoopWire(
	BuildFaceBoundParams		p,
	const TopoDS_Face&			topoAdvancedFace,
	TopoDS_Wire&				topoOuterLoop,
	TopTools_SequenceOfShape&	topoInnerLoops)
{
	TopoDS_Wire loopWire;
	builder.MakeWire(loopWire);
	ShapeFix_Edge edgeFixer;

	TopTools_SequenceOfShape loopEdges;

	for (size_t i = 0; i < p.edges.size(); i++)
	{
		auto edgeParams = p.edges[i];
		TopoDS_Edge topoEdgeCurve = NativeAdvancedFacesBuilder::BuildOrientEdge(edgeParams, edgeFixer);
		if (topoEdgeCurve.IsNull()) {
			continue;
		}
		if (!p.buildRuledSurface)
			edgeFixer.FixAddPCurve(topoEdgeCurve, topoAdvancedFace, false);

		loopEdges.Append(topoEdgeCurve);
	}

	for (auto it = loopEdges.cbegin(); it != loopEdges.cend(); it++)
	{
		builder.Add(loopWire, *it);
	}

	ShapeFix_Wire wireFixer(loopWire, topoAdvancedFace, p.sewingTolerance);
	if (wireFixer.FixReorder())
		loopWire = wireFixer.Wire();
	loopWire.Closed(true);
	BRepCheck_Analyzer analyser(loopWire, Standard_True);

	if (!analyser.IsValid())
	{
		ShapeFix_Wire sfw(loopWire, topoAdvancedFace, p.sewingTolerance);
		if (sfw.Perform())
		{
			loopWire = sfw.Wire();
			loopWire.Checked(true);
		}
	}
	else
		loopWire.Checked(true);

	if (!p.orientation)
	{
		loopWire.Reverse();
	}

	if (p.isOuter)
		topoOuterLoop = loopWire;
	else
		topoInnerLoops.Append(loopWire);
}


TopoDS_Face NativeAdvancedFacesBuilder::BuildFace(BuildFaceParams params, const BRep_Builder& builder)
{
	TopoDS_Wire topoOuterLoop;
	TopTools_SequenceOfShape  topoInnerLoops;
	BRepBuilderAPI_MakeFace faceMaker;
	faceMaker.Init(params.face);

	for (BuildFaceBoundParams faceBoundParams : params.faceBoundsParams) {
		BuildLoopWire(faceBoundParams, params.face, topoOuterLoop, topoInnerLoops);
	}

	if (topoOuterLoop.IsNull())
	{
		double area = 0;
		int foundIndex = -1;
		int idx = 0;
		for (auto it = topoInnerLoops.cbegin(); it != topoInnerLoops.cend(); ++it)
		{
			idx++;
			double loopArea = ShapeAnalysis::ContourArea(TopoDS::Wire(*it));
			if (loopArea > area)
			{
				topoOuterLoop = TopoDS::Wire(*it);
				area = loopArea;
				foundIndex = idx;
			}
		}
		if (foundIndex > 0)topoInnerLoops.Remove(foundIndex); //remove outer loop from inner loops
	}
	if (topoOuterLoop.IsNull())
		return params.face;

	if (params.buildRuledSurface)
	{
		//some models badly define the surface for linear extrusion, is we cannot build the face properly use the filler to create a surface that fits the wire
		//the facemaker is currently intialised for the surface defined in the schema
		//add the loop and check if it fits
		//first see if the surface is within tolerance of the wire loop
		bool buildFromLoop = true;
		if (WithinTolerance(topoOuterLoop, params.face, params.sewingTolerance))
		{
			ShapeFix_Wire wf(topoOuterLoop, faceMaker.Face(), params.sewingTolerance);
			if (wf.FixEdgeCurves())
			{
				topoOuterLoop = wf.Wire();
			}
			faceMaker.Add(topoOuterLoop);
			BRepCheck_Analyzer analyser(faceMaker.Face(), Standard_True);
			buildFromLoop = !analyser.IsValid();
		}
		if (buildFromLoop)
		{
			int edgeCount = 0;
			for (BRepTools_WireExplorer exp(topoOuterLoop); exp.More(); exp.Next()) edgeCount++;
			if (edgeCount == 4) //would indicate a normal ruled surface
			{

				TopTools_ListOfShape curves;

				//get the two curves
				for (BRepTools_WireExplorer exp(topoOuterLoop); exp.More(); exp.Next())
				{
					double first, last;
					Handle(Geom_Curve) curve = BRep_Tool::Curve(exp.Current(), first, last);
					Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(curve);
					if (line.IsNull()) //its a curve
					{
						if (curves.Size() == 1)
						{
							curves.Append(exp.Current().Reversed());
							break;//we only want two curves, the other two should be lines
						}
						else
							curves.Append(exp.Current());
					}
				}
				if (curves.Size() == 2)
				{
					TopoDS_Face ruledFace = BRepFill::Face(TopoDS::Edge(curves.First()), TopoDS::Edge(curves.Last()));
					if (!ruledFace.IsNull())
					{
						ruledFace = TopoDS::Face(ruledFace.EmptyCopied());
						/*if (!advancedFace->SameSense)
							ruledFace.Reverse();*/
						faceMaker.Init(ruledFace);

						ShapeFix_Wire wf2(topoOuterLoop, faceMaker.Face(), params.sewingTolerance);
						if (wf2.Perform())
						{
							topoOuterLoop = wf2.Wire();
						}
						faceMaker.Add(topoOuterLoop);

						buildFromLoop = false; //success
					}
				}
			}
			if (buildFromLoop)
			{
				try
				{
					BRepFill_Filling filler;

					for (BRepTools_WireExplorer exp(topoOuterLoop); exp.More(); exp.Next())
					{
						TopoDS_Edge e = TopoDS::Edge(exp.Current());

						filler.Add(e, GeomAbs_C0);
					}
					filler.Build();
					if (filler.IsDone())
					{
						TopoDS_Face ruledFace = TopoDS::Face(filler.Face().EmptyCopied()); //build with no edges in the resulting face		
						/*if (!advancedFace->SameSense)
							ruledFace.Reverse();*/
						faceMaker.Init(ruledFace);
					}

					ShapeFix_Wire wf2(topoOuterLoop, faceMaker.Face(), params.sewingTolerance);
					if (wf2.Perform())
					{
						topoOuterLoop = wf2.Wire();
					}
					faceMaker.Add(topoOuterLoop);
				}
				catch (Standard_Failure sf)
				{ 
					std::stringstream ss;
					ss << "Could not fill face #";
					ss << std::to_string(params.label);
					ss << ", it has been ignored: ";
					ss << sf.GetMessageString();
					LogWarning(ss.str().c_str());
				}
			}
		}
	}
	else
		faceMaker.Add(topoOuterLoop);

	auto topoAdvancedFace = faceMaker.Face();

	if (topoInnerLoops.Size() > 0) //add any inner bounds
	{
		try
		{
			for (auto it = topoInnerLoops.cbegin(); it != topoInnerLoops.cend(); ++it)
			{
				TopoDS_Wire innerWire = TopoDS::Wire(*it);
				faceMaker.Add(innerWire);
				if (!faceMaker.IsDone()) {
					std::string msg = "Could not apply inner bound to face #" + std::to_string(params.label) + ", it has been ignored";
					LogWarning(msg.c_str());
				}
			}

			ShapeFix_Face fixFaceWire(faceMaker.Face());
			fixFaceWire.FixOrientation();
			topoAdvancedFace = fixFaceWire.Face();
		}
		catch (const Standard_Failure& sf)
		{
			std::string msg = "Could not apply inner bound to face #" + std::to_string(params.label) +
				", it has been ignored: ";
			LogStandardFailure(sf, msg.c_str());
		}
	}

	try
	{

		BRepCheck_Analyzer analyser(topoAdvancedFace, Standard_False);
		if (!analyser.IsValid())
		{
			ShapeFix_Shape sfs(topoAdvancedFace);
			if (sfs.Perform())
			{
				topoAdvancedFace = TopoDS::Face(sfs.Shape());
				topoAdvancedFace.Checked(true);
			}
		}
		else
			topoAdvancedFace.Checked(true);
	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream ss;
		ss << "Fixing face #";
		ss << std::to_string(params.label);
		ss << " failed: ";
		ss << sf.GetMessageString();
		LogWarning(ss.str().c_str());
	}

	return topoAdvancedFace;
}

bool NativeAdvancedFacesBuilder::LocatePointOnCurve(const Handle(Geom_Curve)& C, const TopoDS_Vertex& V, double tolerance, double& p, double& distance)
{
	Standard_Real Eps2 = tolerance * tolerance;

	gp_Pnt P = BRep_Tool::Pnt(V);
	GeomAdaptor_Curve GAC(C);

	// Afin de faire les extremas, on verifie les distances en bout
	Standard_Real D1, D2;
	gp_Pnt P1, P2;
	P1 = GAC.Value(GAC.FirstParameter());
	P2 = GAC.Value(GAC.LastParameter());
	D1 = P1.SquareDistance(P);
	D2 = P2.SquareDistance(P);
	if ((D1 < D2) && (D1 <= Eps2)) {
		p = GAC.FirstParameter();
		distance = sqrt(D1);
		return Standard_True;
	}
	else if ((D2 < D1) && (D2 <= Eps2)) {
		p = GAC.LastParameter();
		distance = sqrt(D2);
		return Standard_True;
	}

	Extrema_ExtPC extrema(P, GAC);
	if (extrema.IsDone()) {
		Standard_Integer i, index = 0, n = extrema.NbExt();
		Standard_Real Dist2 = RealLast(), dist2min;

		for (i = 1; i <= n; i++) {
			dist2min = extrema.SquareDistance(i);
			if (dist2min < Dist2) {
				index = i;
				Dist2 = dist2min;
			}
		}

		if (index != 0) {
			if (Dist2 <= Eps2) {
				p = (extrema.Point(index)).Parameter();
				distance = sqrt(Dist2);
				return Standard_True;
			}
		}
	}
	return Standard_False;
}

bool NativeAdvancedFacesBuilder::WithinTolerance(const  TopoDS_Wire& topoOuterLoop, const TopoDS_Face& topoAdvancedFace, double tolerance)
{
	BRepExtrema_DistShapeShape measure;
	measure.LoadS1(topoAdvancedFace);
	for (TopExp_Explorer exp(topoOuterLoop, TopAbs_EDGE); exp.More(); exp.Next())
	{
		measure.LoadS2(exp.Current());
		bool performed = measure.Perform();
		bool done = measure.IsDone();

		if (!performed || !done || measure.Value() > (tolerance * 10))
		{
			return false;
		}
	}
	return true;
}


TopoDS_Vertex NativeAdvancedFacesBuilder::GetVertex(int label, double x, double y, double z) {
	TopoDS_Vertex vertex;
	if (!vertexGeometries.IsBound(label))
	{
		gp_Pnt startPnt(x, y, z);
		builder.MakeVertex(vertex, startPnt, Precision::Confusion());
		vertexGeometries.Bind(label, vertex);
	}
	else
		vertex = TopoDS::Vertex(vertexGeometries.Find(label));

	return vertex;
}
