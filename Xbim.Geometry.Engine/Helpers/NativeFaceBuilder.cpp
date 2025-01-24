#include "NativeFaceBuilder.h"


TopoDS_Face NativeFaceBuilder::Init(std::vector<gp_Pnt> points, TopoDS_Wire& theWire, bool& sucess)
{
	TColgp_SequenceOfPnt pointSeq;

	for (auto pnt: points)
	{
		pointSeq.Append(pnt);
	}

	if (useVertexMap)
		XbimNativeApi::RemoveDuplicatePoints(pointSeq, handles, true, tolerance);
	else
		XbimNativeApi::RemoveDuplicatePoints(pointSeq, true, tolerance);


	if (pointSeq.Length() != facePointsCount)
	{
		LogInfo("Polyloop with duplicate points. Ifc rule: first point shall not be repeated at the end of the list. It has been removed");
	}

	if (pointSeq.Length() < 3)
	{
		LogWarning("Polyloop with less than 3 points is an empty loop. It has been ignored");
		sucess = false;
		return TopoDS_Face();
	}
	//get the basic properties
	TColgp_Array1OfPnt pointArray(1, pointSeq.Length());
	for (int i = 1; i <= pointSeq.Length(); i++)
	{
		pointArray.SetValue(i, pointSeq.Value(i));
	}

	//limit the tolerances for the vertices and edges
	BRepBuilderAPI_MakePolygon polyMaker;
	for (int i = 1; i <= pointSeq.Length(); ++i) {
		polyMaker.Add(pointSeq.Value(i));
	}
	polyMaker.Close();

	if (polyMaker.IsDone())
	{
		bool isPlanar;
		gp_Vec normal = XbimNativeApi::NewellsNormal(pointArray, isPlanar);
		if (!isPlanar)
		{
			LogWarning("Polyloop is a line. Empty loop built");
			sucess = false;
			return TopoDS_Face();
		}
		gp_Pnt centre = GProp_PGProps::Barycentre(pointArray);
		gp_Pln thePlane(centre, normal);
		theWire = polyMaker.Wire();
		TopoDS_Face theFace = BRepBuilderAPI_MakeFace(thePlane, theWire, false);

		//limit the tolerances for the vertices and edges
		ShapeFix_ShapeTolerance tolFixer;
		tolFixer.LimitTolerance(theWire, tolerance); //set all tolerances
		//adjust vertex tolerances for bad planar fit if we have anything more than a triangle (which will alway fit a plane)

		if (pointSeq.Length() > 3)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(theWire, TopAbs_EDGE, map);
			ShapeFix_Edge ef;
			bool fixed = false;
			for (int i = 1; i <= map.Extent(); i++)
			{
				const TopoDS_Edge edge = TopoDS::Edge(map(i));
				if (ef.FixVertexTolerance(edge, theFace)) fixed = true;

			}
			if (fixed)
				LogInfo("Polyloop is slightly mis-aligned to a plane. It has been adjusted");
		}
		//need to check for self intersecting edges to comply with Ifc rules
		double maxTol = BRep_Tool::MaxTolerance(theWire, TopAbs_VERTEX);
		Handle(ShapeAnalysis_Wire) wa = new ShapeAnalysis_Wire(theWire, theFace, maxTol);

		if (wa->CheckSelfIntersection()) //some edges are self intersecting or not on plane within tolerance, fix them
		{
			ShapeFix_Wire wf;
			wf.Init(wa);
			wf.SetPrecision(tolerance);
			wf.SetMinTolerance(tolerance);
			wf.SetMaxTolerance(maxTol);
			if (!wf.Perform())
			{
				LogWarning("Failed to fix self-interecting wire edges");

			}
			else
			{
				theWire = wf.Wire();
				theFace = BRepBuilderAPI_MakeFace(thePlane, theWire, false);
			}

		}
		sucess = true;
		return theFace;
	}
	else
	{
		LogWarning("Failed to build Polyloop"); //nothing more to say to the log			
	}

	sucess = false;
	return TopoDS_Face();

}


void NativeFaceBuilder::FixFace(TopoDS_Face& theFace, const std::vector<TopoDS_Face>& innerBoundsList, double angularTolerance, ShapeFix_ShapeTolerance& tolFixer, double tolerance)
{
	if (!theFace.IsNull() && innerBoundsList.size() > 0)
	{
		//add the other wires to the face
		BRepBuilderAPI_MakeFace faceMaker(theFace);
		TopoDS_ListOfShape innerBounds;
		for ( auto innerBound : innerBoundsList)
		{
			innerBounds.Append(innerBound);
		}
		TopoDS_ListIteratorOfListOfShape wireIter(innerBounds);
		BRepGProp_Face prop(theFace);
		gp_Pnt centre;
		gp_Vec theFaceNormal;
		double u1, u2, v1, v2;
		prop.Bounds(u1, u2, v1, v2);
		prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, theFaceNormal);

		while (wireIter.More())
		{
			TopoDS_Face face = TopoDS::Face(wireIter.Value());
			BRepGProp_Face fprop(face);
			gp_Vec innerBoundNormalDir;
			fprop.Bounds(u1, u2, v1, v2);
			fprop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, innerBoundNormalDir);

			if (!theFaceNormal.IsOpposite(innerBoundNormalDir, angularTolerance))
			{

				face.Reverse();
			}
			faceMaker.Add(BRepTools::OuterWire(face));
			wireIter.Next();
		}
		theFace = faceMaker.Face();
		//limit the tolerances for the vertices and edges

		tolFixer.LimitTolerance(theFace, tolerance); //set all tolerances
		//adjust vertex tolerances for bad planar fit if we have anything more than a triangle (which will alway fit a plane)


		TopTools_IndexedMapOfShape map;
		TopExp::MapShapes(theFace, TopAbs_EDGE, map);
		ShapeFix_Edge ef;
		bool fixed = false;
		for (int i = 1; i <= map.Extent(); i++)
		{
			const TopoDS_Edge edge = TopoDS::Edge(map(i));
			if (ef.FixVertexTolerance(edge, theFace)) fixed = true;

		}
		if (fixed)
			LogDebug("Face bounds are slightly mis-aligned to a plane. It has been adjusted");


	}
	else
	{
		tolFixer.LimitTolerance(theFace, tolerance); //set all tolerances
	}
}
