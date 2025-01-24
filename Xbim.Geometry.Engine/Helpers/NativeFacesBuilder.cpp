#include "NativeFacesBuilder.h"
#include "XbimNativeApi.h"

TopoDS_Shape NativeFacesBuilder::BuildShell(const std::vector<TopoDS_Face>& faces)
{
	TopoDS_Shell shell;
	builder.MakeShell(shell);

	for (auto face : faces) {
		builder.Add(shell, face);
	}
	
	BRepCheck_Shell checker(shell);
	BRepCheck_Status st = checker.Orientation();
	if (st != BRepCheck_Status::BRepCheck_NoError)
	{
		ShapeFix_Shell shellFixer(shell);
		if (shellFixer.Perform())
		{
			shell = shellFixer.Shell();
			checker.Init(shell);
		}
		if (checker.Closed() == BRepCheck_Status::BRepCheck_NoError)
		{
			shell.Closed(true);
			shell.Checked(true);
			return shell;
		}
		else
		{
			ShapeFix_Shape shapeFixer(shell);
			if (shapeFixer.Perform())
				return shapeFixer.Shape();
			else
				return shell;
		}
	}
	else //it is oriented correctly and closed
	{
		shell.Closed(true);
		shell.Checked(true);
		return shell;
	}
}


TopoDS_Face NativeFacesBuilder::ProcessBounds(const std::vector<FaceBoundParams>& params, bool& success)
{
	TopoDS_Wire outerLoop;
	TopTools_SequenceOfShape innerLoops;

	for (auto bound: params)
	{

		TopoDS_Vertex currentTail;
		BRepBuilderAPI_MakeWire wireMaker;

		for (auto& p : bound.points)
		{
			try
			{
				inspector.ClearResList();
				inspector.SetCurrent(p.Coord());
				vertexCellFilter.Inspect(p.Coord(), inspector);
				TColStd_ListOfInteger results = inspector.ResInd();
				TopoDS_Vertex vertex;
				if (results.Size() > 0) //hit
				{
					//just take the first one as we don't add vertices more than once to a cell
					int vertexIdx = results.First();
					vertex = TopoDS::Vertex(vertices.Value(vertexIdx));
				}
				else //miss
				{
					inspector.Add(p.Coord());
					//build the vertex

					builder.MakeVertex(vertex, p, tolerance);
					vertices.Append(vertex); //it will have the same index as the point in the inspector
					gp_XYZ coordMin = inspector.Shift(p.Coord(), -tolerance);
					gp_XYZ coordMax = inspector.Shift(p.Coord(), tolerance);
					vertexCellFilter.Add(vertices.Size(), coordMin, coordMax);
				}
				if (currentTail.IsNull()) //first one
				{
					currentTail = vertex;
				}
				else if (!currentTail.IsSame(vertex)) //skip if it the same as the last one
				{
					bool sharedEdge = false;
					//make an edge
					if (edgeMap.IsBound(vertex)) //we have an edge that starts at this ones end, it will need to be reversed
					{
						TopTools_ListOfShape edges = edgeMap.Find(vertex);
						for (auto it = edges.cbegin(); it != edges.cend(); it++)
						{
							TopoDS_Edge edge = TopoDS::Edge(*it);
							TopoDS_Vertex edgeEnd = TopExp::LastVertex(edge, false); //it will laways be forward oriented
							if (edgeEnd.IsSame(currentTail)) //we want this edge reversed
							{
								wireMaker.Add(TopoDS::Edge(edge.Reversed()));
								sharedEdge = true;
								break;
							}
						}
					}
					if (!sharedEdge && edgeMap.IsBound(currentTail)) //we have an edge that starts at this ones end
					{
						TopTools_ListOfShape edges = edgeMap.Find(currentTail);
						for (auto it = edges.cbegin(); it != edges.cend(); it++)
						{
							TopoDS_Edge edge = TopoDS::Edge(*it);
							TopoDS_Vertex edgeEnd = TopExp::LastVertex(edge, false); //it will laways be forward oriented
							if (edgeEnd.IsSame(vertex)) //we want this edge 
							{
								wireMaker.Add(TopoDS::Edge(edge));
								sharedEdge = true;
								break;
							}
						}
					}
					if (!sharedEdge) //make and add the new forward oriented edge if we have not found one
					{
						TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(currentTail, vertex);
						wireMaker.Add(edge);
						if (edgeMap.IsBound(currentTail)) //add it to the list
						{
							edgeMap.ChangeFind(currentTail).Append(edge);
						}
						else //create a new list
						{
							TopTools_ListOfShape edges;
							edges.Append(edge);
							edgeMap.Bind(currentTail, edges);
						}
					}
					currentTail = vertex; //move the tail on
				}
			}
			catch (const Standard_Failure& sf)
			{
				LogStandardFailure(sf);
				continue;
			}
		}

		if (!wireMaker.IsDone()) //if its not the first point its gone wrong
		{
			LogDebug("Empty loop built and ignored");
			continue;
		}
		else
		{
			TopoDS_Wire wire = wireMaker.Wire();
			if (!bound.orientation) wire.Reverse();
			if (bound.isOuter)
				outerLoop = wire;
			else
			{
				innerLoops.Append(wire);
			}
		}
	}

	if (outerLoop.IsNull())
	{
		double area = 0;
		int foundIndex = -1;
		int idx = 0;
		for (auto it = innerLoops.cbegin(); it != innerLoops.cend(); ++it)
		{
			idx++;
			double loopArea = ShapeAnalysis::ContourArea(TopoDS::Wire(*it));
			if (loopArea > area)
			{
				outerLoop = TopoDS::Wire(*it);
				area = loopArea;
				foundIndex = idx;
			}
		}
		if (foundIndex > 0)innerLoops.Remove(foundIndex); //remove outer loop from inner loops
	}
	if (outerLoop.IsNull())
	{
		//no bounded face
		LogDebug("No outer loop built,  face ignored");
		success = false;
		return TopoDS_Face();
	}

	try
	{
		bool isValidNormal;
		gp_Dir outerNormal = XbimNativeApi::NormalDir(outerLoop, isValidNormal); //this can throw an exception if the wire is nonsense (line) and should just be dropped
		if (!isValidNormal)
			Standard_Failure::Raise("Outer bound has invalid normal");
		TopoDS_Vertex v1, v2;
		TopExp::Vertices(outerLoop, v1, v2);
		gp_Pln thePlane(BRep_Tool::Pnt(v1), outerNormal);
		BRepBuilderAPI_MakeFace faceMaker(thePlane, outerLoop, true);
		if (faceMaker.IsDone())
		{
			if (innerLoops.Size() > 0)
			{
				for (auto it = innerLoops.cbegin(); it != innerLoops.cend(); ++it)
				{
					//ensure it is the correct orientation
					try
					{
						TopoDS_Wire innerWire = TopoDS::Wire(*it);
						gp_Vec innerNormal = XbimNativeApi::NormalDir(innerWire, isValidNormal);
						if (!isValidNormal) Standard_Failure::Raise("Inner bound has invalid normal");
						if (!outerNormal.IsOpposite(innerNormal, 0.1))
							innerWire.Reverse();
						faceMaker.Add(innerWire);
					}
					catch (const Standard_Failure& /*sf*/)
					{
						LogDebug("Inner wire has invalid normal,  bound ignored");
						continue;
					}
				}
			}
			success = true;

			return faceMaker.Face();
		}
		else
		{
			LogDebug("Face could not be built, face ignored");
		}
	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream strm;
		sf.Print(strm);
		System::String^ msg = gcnew System::String(strm.str().c_str());
		LogStandardFailure(sf, "Outer loop is not a bounded area,  face ignored: ");
	}

	success = false;
	return TopoDS_Face();
}