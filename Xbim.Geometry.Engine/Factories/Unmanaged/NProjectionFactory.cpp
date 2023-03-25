#include "NProjectionFactory.h"

#include <HLRBRep_HLRToShape.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepMesh_Vertex.hxx>
#include <BRepBndLib.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>

#include <Poly_Polygon2D.hxx>
#include <BRep_CurveOnSurface.hxx>
#include <gp_Dir2d.hxx>
#include <Geom2d_Line.hxx>
#include <TColGeom2d_SequenceOfCurve.hxx>
#include <BRepMesh_VertexInspector.hxx>

#include <algorithm>
#include <HLRAlgo_Projector.hxx>
#include <HLRBRep_Algo.hxx>
#include <IMeshTools_Parameters.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include "NShapeFactory.h"
#include <TopoDS.hxx>
#include <Geom_Plane.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <gp_Ax2.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include "NLoopSegment.h"
#include <unordered_set>
#include <BRepTools.hxx>

int NProjectionFactory::GetOrAddVertex(
	BRepMesh_VertexInspector& anInspector,
	VertexCellFilter& theCells,
	//std::map<int, std::set<int>>& arcs,
	double x,
	double y,
	double tolerance,
	bool& isNew)

{

	gp_XY vertex(x, y);
	gp_XY minVertex(x - tolerance, y - tolerance);
	gp_XY maxVertex(x + tolerance, y + tolerance);
	anInspector.SetPoint(vertex);
	theCells.Inspect(minVertex, maxVertex, anInspector);
	int id = anInspector.GetCoincidentPoint();
	isNew = (id <= 0);
	if (isNew) //its not in the vertex content we need to create it
	{
		id = anInspector.NbVertices() + 1; //increment as internally the inspectors vectors are 1 not 0 based
		BRepMesh_Vertex keyedVertex(vertex, id, BRepMesh_DegreeOfFreedom::BRepMesh_OnSurface);
		anInspector.Add(keyedVertex);
		theCells.Add(id, minVertex, maxVertex);
	}

	return id;
}
bool NProjectionFactory::UpdateLeftMostPoint(gp_XY& leftMost, double x, double y, double tolerance)
{
	if (x <= leftMost.X()) //its a candidate for left most, check if it is equal and lowest
	{
		double xDiff = std::abs(leftMost.X() - x);
		if (xDiff <= tolerance) //the Xs are the same, check the Ys
		{
			if (y < leftMost.Y()) //only adjust if the point is lower
			{
				leftMost.SetX(x);
				leftMost.SetY(y);
				return true;
			}
		}
		else  //we have a new left most point
		{
			leftMost.SetX(x);
			leftMost.SetY(y);
			return true;
		}
	}
	return false;
}

void NProjectionFactory::FindOuterLoops(BRepMesh_VertexInspector& anInspector, std::map<int, std::set<int>>& arcs, double linearDeflection, double angularDeflection, double tolerance, NFootprint& footprint)
{

	if (arcs.size() == 0) return;
	const double twoPi = std::_Pi * 2;


	//start at the lowest, leftist point and proceed anticlockwise around the perimeter
	//start with the direction being downward, of the left most point, measure angle counter next segment clockwise from downward, the smallest angle will be the outbound segment we want

	auto&& arcIt = arcs.cbegin();
	gp_XY currentPoint = anInspector.GetVertex(arcIt->first).Coord();
	gp_XY previousPoint;
	int currentPointId = arcIt->first;
	gp_XY leftMostPoint = currentPoint;
	int leftMostPointIndex = currentPointId;
	//find the leftmost lowest point
	for (; arcIt != arcs.cend(); arcIt++)
	{
		gp_XY point = anInspector.GetVertex(arcIt->first).Coord();
		if (UpdateLeftMostPoint(leftMostPoint, point.X(), point.Y(), tolerance))
			leftMostPointIndex = arcIt->first;
	}
	currentPointId = leftMostPointIndex;
	currentPoint = leftMostPoint;
	gp_Dir2d currentDir(0, -1);
	gp_Dir2d previousDir;
	std::vector<int> outerBound;

	outerBound.push_back(currentPointId);
	int previousPointId = 0;
	//nb we never want to traverse a segment more than once as this will create a recursion
	std::unordered_set<NLoopSegment> outerBoundSegments;
	do
	{
		const std::set<int>& toPoints = arcs[currentPointId]; //get the target points for this segment arc start

		if (toPoints.size() == 1)
		{
			int nextId = *(toPoints.cbegin());
			gp_XY pt = anInspector.GetVertex(nextId).Coord();
			if (nextId == previousPointId) //we are going backwards to the point we came from and there is no alternative to go forward, 
				//reset to the previous point, the outerBoundSegments set will have recorded this traversal so it will be ignored next time
			{
				//consider possibility that the first point has size of 1
				if(outerBound.size()==0)
					Standard_Failure::Raise("Footprint is an invalid, boundary cannot be determined");
				currentPointId = previousPointId;
				currentPoint = previousPoint;
				currentDir = previousDir;
				continue;
				
			}
			//remove the current point and arcs as we have processed them
			//arcs.erase(currentPointId);

			previousPointId = currentPointId;
			previousPoint = currentPoint;
			currentPointId = nextId;
			previousDir = currentDir;
			currentPoint = anInspector.GetVertex(nextId).Coord(); //when we lookup the vertices internally they are 0 based
			currentDir = currentPoint - previousPoint;
			outerBound.push_back(currentPointId);
		}
		else
		{
			double minAngle = twoPi;
			gp_XY nextPoint;
			int nextPointId = 0;
			for (auto&& pntId : toPoints)
			{

				if (pntId == previousPointId || currentPointId == pntId) //we are going backwards or nowhere, skip it
					continue;
				//check if the segment has already been traversed, ignore if it has
				if (auto find = outerBoundSegments.find(NLoopSegment(currentPointId, pntId)); find != outerBoundSegments.cend())
					continue;

				gp_XY nextCandidatePoint = anInspector.GetVertex(pntId).Coord();

				gp_Vec2d nextDir = nextCandidatePoint - currentPoint;
				double angle = currentDir.Angle(nextDir);// +std::_Pi; //range is 0 to <2Pi in radians
				if (angle <= 0) //adjust to range 0 -> 2Pi
					angle = twoPi + angle;

				if (angle < minAngle) //this is a candidate for min clockwise angle
				{
					nextPoint = nextCandidatePoint;
					nextPointId = pntId;
					minAngle = angle;
				}

			}
			//if we do't have a next valid point throw exception
			if (nextPointId == 0 || nextPointId == currentPointId)
			{
				Standard_Failure::Raise("Footprint is an invalid, boundary cannot be determined");
			}
			else
			{
				outerBoundSegments.insert(NLoopSegment(currentPointId, nextPointId));
				outerBound.push_back(nextPointId);
				previousPointId = currentPointId;
				previousPoint = currentPoint;
				previousDir = currentDir;
				currentDir = currentPoint - nextPoint; //reverse the direction to ensure counter clockwise angles
				currentPoint = nextPoint;
				currentPointId = nextPointId;
			}


		}
	} while (currentPointId != leftMostPointIndex); //test if we are back at the start of the bound

	//find all the points connected to points in this boundary, these can be ignored
	std::set<int> connected;
	ConnectedPoints(leftMostPointIndex, arcs, connected);
	for (auto&& arc : connected)
		arcs.erase(arc);

	//record the boundary as a closed polygon
	TColgp_Array1OfPnt2d arrayOfPnt2d(1, (int)outerBound.size());
	int i = 1;
	for (auto&& vertex : outerBound)
	{
		arrayOfPnt2d.SetValue(i++, anInspector.GetVertex(vertex).Coord());
	}


	//push to the footprint
	//check the bound is  internal, we are going to ignore all internal loops as for the majority of cases they are not representative
	//for example, if we have a vertical hollow cylinder, do shapes inside the cyliner count as intersections;
	//if the cylinder is slightly off vertival is the hole a valid representation of the internal void?
	//it might be wise to chack the area of the loop is positve and > 0, but the calculation method above of the loop should avoid this possibility
	if (!footprint.IsHole(arrayOfPnt2d))
	{
		//add to the footprint
		Handle(Poly_Polygon2D) polygon = new Poly_Polygon2D(arrayOfPnt2d);
		std::vector<Handle(Poly_Polygon2D)> rings;
		rings.push_back(polygon);
		footprint.Bounds.push_back(rings);
	}


	if (arcs.size() > 0) //recurse other loops if we have any arcs left to inspect
		FindOuterLoops(anInspector, arcs, linearDeflection, angularDeflection, tolerance, footprint);

}




void NProjectionFactory::CreateFootPrint(const TopoDS_Shape& shape, double linearDeflection, double angularDeflection, double tolerance, NFootprint& footprint)
{

	const double halfPi = std::_Pi / 2;
	//find the bounds
	Bnd_Box box;

	BRepBndLib::AddClose(shape, box);
	Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
	if (box.IsVoid())
	{
		pLoggingService->LogWarning("Cannot build Footprint shape is empty");
		return; //untouched footprint
	}
	box.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);

	try
	{
		//process each 
		HLRAlgo_Projector aProjector; //create an axonometric projector with 0 focus and a plane it the XY plane at height srZmin

		Handle(HLRBRep_Algo) aHlrBrepAlgo = new HLRBRep_Algo();
		//project the shape onto plane Z==0

		aHlrBrepAlgo->Add(shape);
		aHlrBrepAlgo->Projector(aProjector);
		aHlrBrepAlgo->Update();
		aHlrBrepAlgo->Hide();

		HLRBRep_HLRToShape aBrepHlr2Shape(aHlrBrepAlgo);

		//get the visible and outline line segments


		IMeshTools_Parameters meshParams;
		meshParams.Deflection = linearDeflection;
		meshParams.Angle = angularDeflection;

		TopTools_SequenceOfShape visibleEdges;

		for (TopExp_Explorer e(aBrepHlr2Shape.CompoundOfEdges(HLRBRep_TypeOfResultingEdge::HLRBRep_Sharp, true, false), TopAbs_EDGE); e.More(); e.Next())
		{
			//triangulate the edge to get the polygon we want to represent it
			NShapeFactory::Triangulate(e.Current(), meshParams);
			visibleEdges.Append(e.Current());
		}

		//sometimes outline curves can intersect, split the curves and check for intersection as a bound of a shape cannot have intersecting edges
		TopTools_SequenceOfShape outlineEdges;
		for (TopExp_Explorer e(aBrepHlr2Shape.CompoundOfEdges(HLRBRep_TypeOfResultingEdge::HLRBRep_OutLine, true, false), TopAbs_EDGE); e.More(); e.Next())
		{
			//triangulate the edge to get the polygon we want to represent it
			NShapeFactory::Triangulate(e.Current(), meshParams);
			outlineEdges.Append(e.Current());
		}

		//transform all the edge into linear segments
		TColGeom2d_SequenceOfCurve segments;
		for (int i = 1; i <= outlineEdges.Size(); i++)
		{
			TopoDS_Edge edge = TopoDS::Edge(outlineEdges.Value(i));
			ConvertToLinearSegments(edge, segments, tolerance);
		}
		//turn the visible edges to segments too
		for (int i = 1; i <= visibleEdges.Size(); i++)
		{
			TopoDS_Edge edge = TopoDS::Edge(visibleEdges.Value(i));
			ConvertToLinearSegments(edge, segments, tolerance);
		}
		//ensure that no segments intersect, essential for determining the correct outer boundary
		//NB this code may be optimised for large objects by using octrees, 
		std::map<int, std::vector<double>> segmentSplits; //key is the segment index in "segments" value is the U parameter of the interesction on the line
		for (int i = 1; i <= segments.Size(); i++)
		{
			Handle(Geom2d_TrimmedCurve) segmentA = Handle(Geom2d_TrimmedCurve)::DownCast(segments.Value(i));
			for (int j = i + 1; j <= segments.Size(); j++)
			{
				Handle(Geom2d_TrimmedCurve) segmentB = Handle(Geom2d_TrimmedCurve)::DownCast(segments.Value(j));
				//check if there are any interections between this segment pair
				Geom2dAPI_InterCurveCurve curveInter(segmentA, segmentB, tolerance);
				int numIntersections = curveInter.NbPoints();
				for (int p = 1; p <= numIntersections; p++)
				{

					//u value will be the distance between the first point and this point
					gp_Pnt2d startA = segmentA->StartPoint();
					double distA = startA.Distance(curveInter.Point(p));
					bool contiguousSegmentA = (std::abs(distA) < tolerance || std::abs(segmentA->FirstParameter() + distA - segmentA->LastParameter()) < tolerance);
					//ignore if the intersection is at either end of the segment (this happens with spheres etc)
					if (!contiguousSegmentA)
					{
						double u = segmentA->FirstParameter() + distA;
						//ensure we are inserted with an empty set
						auto&& result = segmentSplits.emplace(i, std::vector<double>());
						result.first->second.push_back(u);
					}
					//u value will be the distance between the first point and this point
					gp_Pnt2d startB = segmentB->StartPoint();
					double distB = startB.Distance(curveInter.Point(p));
					bool contiguousSegmentB = (std::abs(distB) < tolerance || std::abs(segmentB->FirstParameter() + distB - segmentB->LastParameter()) < tolerance);
					//ignore if the intersection is at either end of the segment (this happens with spheres etc)
					if (!contiguousSegmentB)
					{
						double u = segmentB->FirstParameter() + distB;
						//do the other segment
						auto&& result = segmentSplits.emplace(j, std::vector<double>());
						result.first->second.push_back(u);
					}
				}
			}
		}

		//apply any splits to the appropriate segments
		for (auto&& splitSegIt = segmentSplits.cbegin(); splitSegIt != segmentSplits.cend(); splitSegIt++)
		{
			//append the new segments
			int segmentIndex = splitSegIt->first;
			Handle(Geom2d_TrimmedCurve) segment = Handle(Geom2d_TrimmedCurve)::DownCast(segments.Value(segmentIndex));
			//sort the u values so we chop in the correct order
			auto splitParams = splitSegIt->second;
			std::sort(splitParams.begin(), splitParams.end());
			double start = segment->FirstParameter();
			auto& splitsIt = splitParams.cbegin();
			double end = *splitsIt;
			if (start == end)
				continue;
			Handle(Geom2d_TrimmedCurve) shortenedSegment = new Geom2d_TrimmedCurve(segment->BasisCurve(), start, end);
			segments.SetValue(segmentIndex, shortenedSegment);
			splitsIt++; //we have used the first one
			for (; splitsIt != splitParams.cend(); splitsIt++) //go through intermediate segments
			{
				start = end;
				end = *splitsIt;
				if (start == end)
					continue;
				Handle(Geom2d_TrimmedCurve) newSegment = new Geom2d_TrimmedCurve(segment->BasisCurve(), start, end);
				segments.Append(newSegment);
			}
			//need to add the last segment in if we have some left
			if (std::abs(end - segment->LastParameter()) > tolerance)
			{
				Handle(Geom2d_TrimmedCurve) lastSegment = new Geom2d_TrimmedCurve(segment->BasisCurve(), end, segment->LastParameter());
				segments.Append(lastSegment);
			}
		}



		if (segments.Size() == 0)
		{
			const char* msg = "No lines segments found to build Footprint";
			pLoggingService->LogWarning(msg);
			Standard_Failure::Raise(msg);
		}

		//Create unique points for all the segments, shared points where coincidental
		//create a grid of cells  this is the typical linear deflection for triangulation
		//set the cell size to 10 times tolerance to speed up searching
		BRepMesh_VertexInspector anInspector(new NCollection_IncAllocator);
		VertexCellFilter theCells(2, 10 * tolerance, new NCollection_IncAllocator);
		std::map<int, std::set<int>> arcs;
		anInspector.SetTolerance(tolerance);


		for (auto&& segIt : segments)
		{
			Handle(Geom2d_TrimmedCurve) seg = Handle(Geom2d_TrimmedCurve)::DownCast(segIt);
			gp_Pnt2d pointA = seg->StartPoint();
			gp_Pnt2d pointB = seg->EndPoint();

			bool isNewA, isNewB;
			int idA = GetOrAddVertex(anInspector, theCells, pointA.X(), pointA.Y(), tolerance, isNewA);
			int idB = GetOrAddVertex(anInspector, theCells, pointB.X(), pointB.Y(), tolerance, isNewB);
			if (idA != idB)// no point in adding a line segment of 0 length
			{
				if (isNewA) arcs[idA] = std::set<int>{ idB }; else arcs[idA].insert(idB);
				if (isNewB) arcs[idB] = std::set<int>{ idA }; else arcs[idB].insert(idA);
			}
			pointA = pointB;

		}
		//get the boundaries and rings
		FindOuterLoops(anInspector, arcs, linearDeflection, angularDeflection, tolerance, footprint);
		//remove any colinear lines
		footprint.SimplifyBounds();

		footprint.MinZ = srZmin;
		footprint.MaxZ = srZmax;
		footprint.IsClose = true;
	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream strm;
		sf.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		//return the bounding box
		TColgp_Array1OfPnt2d arrayOfPnt2d(1, 5);
		arrayOfPnt2d.SetValue(1, gp_Pnt2d(srXmin, srYmin));
		arrayOfPnt2d.SetValue(2, gp_Pnt2d(srXmax, srYmin));
		arrayOfPnt2d.SetValue(3, gp_Pnt2d(srXmax, srYmax));
		arrayOfPnt2d.SetValue(4, gp_Pnt2d(srXmin, srYmax));
		arrayOfPnt2d.SetValue(5, gp_Pnt2d(srXmin, srYmin));
		Handle(Poly_Polygon2D) polygon = new Poly_Polygon2D(arrayOfPnt2d);
		std::vector<Handle(Poly_Polygon2D)> rings;
		rings.push_back(polygon);
		footprint.Bounds.push_back(rings);
		footprint.IsClose = false;
		footprint.MinZ = srZmin;
		footprint.MaxZ = srZmax;
	}

}

void NProjectionFactory::ConvertToLinearSegments(const TopoDS_Edge& edge, TColGeom2d_SequenceOfCurve& segments, double tolerance)
{
	TopLoc_Location location;

	Handle(Poly_Polygon3D) polygon3d = BRep_Tool::Polygon3D(edge, location);
	if (polygon3d->NbNodes() == 0) return; // nothing here	
	bool isIdentity = location.IsIdentity();
	gp_Trsf matrix;
	//this should be unnecessary and never happen, as with projection code it is always identity; protective measure in case OCC code changes in the future
	if (!isIdentity) matrix = location.Transformation();
	auto&& segIt = polygon3d->Nodes().cbegin(); //grab the first point
	gp_XYZ pointA = segIt->XYZ();
	if (!isIdentity) matrix.Transforms(pointA);
	gp_XYZ firstPoint = pointA;
	segIt++; //move to next point
	for (; segIt != polygon3d->Nodes().cend(); segIt++)
	{
		gp_XYZ pointB = segIt->XYZ();
		if (!isIdentity) matrix.Transforms(pointB);
		Handle(Geom2d_TrimmedCurve) segment;
		if (BuildSegment(pointA, pointB, tolerance, segment))
			segments.Append(segment);
		pointA = pointB;
	}
	if (edge.Closed()) //for a closed segment like a circle or elipse, we need to repeat the last segment
	{
		Handle(Geom2d_TrimmedCurve) segment;
		if (BuildSegment(pointA, firstPoint, tolerance, segment))
			segments.Append(segment);
	}
}

bool NProjectionFactory::BuildSegment(gp_XYZ pointA, gp_XYZ pointB, double tolerance, Handle(Geom2d_TrimmedCurve)& segment)
{
	// z is always 0 in this situation, due to the  selected projection plane
	gp_XY a(pointA.X(), pointA.Y()); gp_XY b(pointB.X(), pointB.Y());
	if (a.IsEqual(b, tolerance)) false; //ignore zero length segments
	gp_Vec2d dir = b - a;
	Handle(Geom2d_Line) segLine = new Geom2d_Line(a, dir);
	segment = new Geom2d_TrimmedCurve(segLine, 0, dir.Magnitude());
	return true;
}


TopoDS_Compound NProjectionFactory::GetOutline(const TopoDS_Shape& shape)
{
	HLRAlgo_Projector aProjector; //create an axonometric projector with 0 focus and a plane it the XY plane at height srZmin

	Handle(HLRBRep_Algo) aHlrBrepAlgo = new HLRBRep_Algo();
	HLRBRep_HLRToShape aBrepHlr2Shape(aHlrBrepAlgo);

	aHlrBrepAlgo->Add(shape);
	aHlrBrepAlgo->Projector(aProjector);
	aHlrBrepAlgo->Update();
	aHlrBrepAlgo->Hide();

	BRep_Builder b;
	TopoDS_Compound c;
	b.MakeCompound(c);

	for (TopExp_Explorer e(aBrepHlr2Shape.CompoundOfEdges(HLRBRep_TypeOfResultingEdge::HLRBRep_Sharp, true, false), TopAbs_EDGE); e.More(); e.Next())
	{
		b.Add(c, e.Current());
	}
	for (TopExp_Explorer e(aBrepHlr2Shape.CompoundOfEdges(HLRBRep_TypeOfResultingEdge::HLRBRep_OutLine, true, false), TopAbs_EDGE); e.More(); e.Next())
	{
		b.Add(c, e.Current());
	}

	return c;

}

bool NProjectionFactory::CreateSection(const TopoDS_Shape& shape, const Handle(Geom_Surface)& cutSurface, double tolerance, TopTools_ListOfShape& result)
{
	try
	{
		//build the cutting face
		TopoDS_Face cutFace = BRepBuilderAPI_MakeFace(cutSurface,
			Standard_Real::MinValue,
			Standard_Real::MaxValue,
			Standard_Real::MinValue,
			Standard_Real::MaxValue,
			tolerance);
		BRepAlgoAPI_Section boolOp;
		TopTools_ListOfShape tools;
		tools.Append(cutFace);
		TopTools_ListOfShape args;
		args.Append(shape);
		boolOp.SetTools(tools);
		boolOp.SetArguments(args);
		boolOp.Build();
		if (!boolOp.IsDone())
			Standard_Failure::Raise("Could not build section");
		//create a list of unconnected edges
		const TopTools_ListOfShape& edgeList = boolOp.SectionEdges();

		//create a set of closed and open wires, we will only consider closed wires
		TopoDS_Compound closedWires = FindClosedWires(edgeList, tolerance);

		BRepAlgo_FaceRestrictor fr;
		fr.Init(cutFace, Standard_True, Standard_True);

		for (TopExp_Explorer exp(closedWires, TopAbs_WIRE); exp.More(); exp.Next())
		{
			TopoDS_Wire wire = TopoDS::Wire(exp.Current());
			fr.Add(wire);
		}
		fr.Perform();
		if (fr.IsDone())
		{
			for (; fr.More(); fr.Next())
			{
				TopoDS_Face face = fr.Current();
				face.Closed(true);
				result.Append(face);
			}
			return true;
		}

	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream strm;
		sf.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogError("Failed to create section");
	return false;
}

TopoDS_Compound NProjectionFactory::FindClosedWires(const TopTools_ListOfShape& edgeList, double tolerance)
{
	Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape;
	Handle(TopTools_HSequenceOfShape) wires;
	TopoDS_Compound open, closed;
	BRep_Builder b;
	b.MakeCompound(open);
	b.MakeCompound(closed);

	for (auto&& edgeIt = edgeList.cbegin(); edgeIt != edgeList.cend(); edgeIt++)
		edges->Append(*edgeIt);

	ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance, false, wires);
	ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);

	return closed;
}

/// <summary>
/// inspects all points in arcs to find the bearest point that is an outer segment
/// </summary>
/// <param name="currentPointIndex"></param>
/// <param name="anInspector"></param>
/// <param name="arcs"></param>
/// <returns></returns>
int NProjectionFactory::FindNextNearestOuterPoint(int currentPointIndex, int previousPointIndex, gp_XY currentPoint, gp_Dir2d currentDir, BRepMesh_VertexInspector& anInspector,
	const std::map<int, std::set<int>>& arcs)
{

	double twoPi = std::_Pi * 2;
	double minAngle = twoPi;
	double minDistance = 10000;
	int nextOuterPoint = 0;

	for (auto&& arcIt : arcs) //check every point
	{
		int pointId = arcIt.first;
		if (pointId != currentPointIndex && pointId != previousPointIndex) //ignore any points on the current segment
		{
			gp_XY point = anInspector.GetVertex(pointId).Coord();
			gp_Vec2d nextDir = point - currentPoint;
			double angle = currentDir.Angle(nextDir);// +std::_Pi; //range is 0 to <2Pi in radians
			if (angle < 0) //adjust to range 0 -> 2Pi
				angle = twoPi + angle;
			if (angle <= minAngle)
			{
				double distanceAway = nextDir.Magnitude();
				if (std::abs(angle - minAngle) < Precision::Angular()) //same angles check the distance
				{

					if (distanceAway < minDistance) //update the angles and distance if this is a nearer point
					{
						minAngle = angle;
						minDistance = distanceAway;
						nextOuterPoint = pointId;
					}
				}
				else
				{
					minAngle = angle;
					minDistance = distanceAway;
					nextOuterPoint = pointId;
				}
			}
		}
	}
	return nextOuterPoint;
}

/// <summary>
/// Returns a set of point indexes that are directly or indirectly connected to the connectedTo point.
/// </summary>
/// <param name="connectedTo"></param>
/// <param name="arcs"></param>
void NProjectionFactory::ConnectedPoints(int connectedTo, const std::map<int, std::set<int>>& arcs, std::set<int>& connected)
{
	auto&& insertResult = connected.insert(connectedTo);
	if (insertResult.second) //we have not previously inserted this element
	{
		auto&& segs = arcs.find(connectedTo);
		for (auto&& arcIt : segs->second)
		{
			//auto res = connected.insert(*arcIt);
			//if (res.second) //we have not previously inserted this element
			//{
			ConnectedPoints(arcIt, arcs, connected);
			//}
		}
	}

}