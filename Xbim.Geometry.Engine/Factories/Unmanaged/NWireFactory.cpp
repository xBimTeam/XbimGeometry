#include "NWireFactory.h"
#include <TopoDS.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Ax2.hxx>
#include <Bnd_Box.hxx>
#include <NCollection_CellFilter.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <Precision.hxx>
#include <set>
#include <ShapeAnalysis_Wire.hxx>
#include <Bnd_Box2d.hxx>
#include <BRepMesh_VertexInspector.hxx>
#include <stdio.h>
#include <IntAna2d_AnaIntersection.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <ShapeFix_Wire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <gp_Pln.hxx>
#include <ShapeFix_IntersectionTool.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <BRepBuilderAPI_VertexInspector.hxx>
#include <BRepBuilderAPI_CellFilter.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRep_Tool.hxx>
#include <GC_MakeSegment.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Circ.hxx>
#include <GCE2d_MakeCircle.hxx>
#include <gp_Circ2d.hxx>
#include <GC_MakeCircle.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <ShapeAnalysis.hxx>

#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2d_Line.hxx>
#include <ShapeFix_Edge.hxx>
#include <BRepLib.hxx>
#include <Geom_Plane.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <GeomLib_Tool.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>


bool NWireFactory::IsClosed(const TopoDS_Wire& wire, double tolerance)
{
	try
	{
		BRepAdaptor_CompCurve cc(wire, Standard_True);
		gp_Pnt p1 = cc.Value(cc.FirstParameter());
		gp_Pnt p2 = cc.Value(cc.LastParameter());
		return p1.IsEqual(p2, tolerance);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
		return false;
	}
}
TopoDS_Wire NWireFactory::BuildWire(const TopoDS_Edge& edge)
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

TopoDS_Wire NWireFactory::BuildWire(const TopTools_SequenceOfShape& edgeList)
{
	try
	{
		BRep_Builder builder;
		TopoDS_Wire loopWire;
		builder.MakeWire(loopWire);

		for (const auto& edge : edgeList)
		{
			builder.Add(loopWire, TopoDS::Edge(edge));
		}
		return loopWire;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildPolyline2d(const TColgp_Array1OfPnt2d& points, double tolerance, bool& hasInfo)
{
	//we need to ensure that no points are technically duplicate, i.e. < tolerance of the model apart
	//the method of removing duplicates that are tolerance away from each other is flawed when the polyline is used
	//in a shared vertex context such as a brep definition, using cell filters and controlling precision is better
	try
	{
		hasInfo = false;
		int id = 0;
		NCollection_Vector<KeyedPnt2d> pointSeq;
		for (auto&& point : points)
		{
			pointSeq.Append(KeyedPnt2d(point.XY(), ++id));
		}

		BRep_Builder builder;
		NCollection_CellFilter<BRepMesh_VertexInspector> theCells(tolerance, new NCollection_IncAllocator);
		gp_Pnt2d start = pointSeq.cbegin()->myPnt2d;
		gp_Pnt2d end = pointSeq.Last().myPnt2d;
		double d = start.Distance(end);

		bool closed = d <= tolerance; //check if the polyline is specified as closed (end repeats the start point)

		int myLastId = -1;
		bool warnedOfSelfIntersection = false;
		BRepMesh_VertexInspector anInspector(new NCollection_IncAllocator);
		anInspector.SetTolerance(tolerance);

		TopTools_SequenceOfShape vertices;
		int pCount = pointSeq.Size();
		for (auto&& keyedPnt : pointSeq)
		{
			pCount--;
			gp_XY pnt = keyedPnt.myPnt2d;
			gp_XY pMin = gp_XY(pnt.X() - tolerance, pnt.Y() - tolerance);
			gp_XY pMax = gp_XY(pnt.X() + tolerance, pnt.Y() + tolerance);
			anInspector.SetPoint(pnt);
			theCells.Inspect(pMin, pMax, anInspector);
			int aResID = anInspector.GetCoincidentPoint();

			if (aResID <= 0) //its not in the vertex content we need to create it
			{
				BRepMesh_Vertex keyedVertex(pnt, keyedPnt.myID, BRepMesh_DegreeOfFreedom::BRepMesh_OnCurve);
				myLastId = anInspector.Add(keyedVertex);
				theCells.Add(myLastId, pMin, pMax);
				vertices.Append(keyedPnt.CreateTopoVertex());
			}
			else //we have a coincidental vertex from the context
			{
				//adjust its tolerance and position so that the shared vertex is a surrogate for both regardless 
				const BRepMesh_Vertex& meshVert = anInspector.GetVertex(aResID);
				gp_XY foundPoint = meshVert.Coord();
				gp_Vec2d vecBetween(foundPoint, pnt); //vector between the found point and the inspected point
				gp_Vec2d displacement = vecBetween.Divided(2);//get the vector to move to a point half way between the two
				foundPoint.Add(displacement.XY());
				const TopoDS_Vertex& kv = TopoDS::Vertex(vertices.Value(aResID));
				double toleranceOfFound = BRep_Tool::Tolerance(kv);
				double distanceFromFound = displacement.Magnitude(); //distance from the mid point to the found point
				double requiredTolerance = std::max(distanceFromFound + toleranceOfFound, distanceFromFound + Precision::Confusion());
				builder.UpdateVertex(kv, gp_Pnt(foundPoint.X(), foundPoint.Y(), 0), requiredTolerance); //make the found point a surrogate for both points
				if (myLastId == aResID) //its the same as the previous point, just ignore and carry on
				{
					hasInfo = true;
					continue;
				}
				else if (pCount != 0) //we are adding a point we already have connnected and we are not on the last point
				{
					if (!warnedOfSelfIntersection)
					{
						pLoggingService->LogDebug("Self intersecting polyline");
						hasInfo = true;
						warnedOfSelfIntersection = true; //just do it once
					}

				}
				vertices.Append(kv);
				myLastId = aResID;
			}

		}
		//The inspector now has our final vertex list
		int desiredPointCount = pointSeq.Length();
		int actualPointCount = vertices.Size();
		if (actualPointCount < desiredPointCount) //we have removed duplicate points
		{
			//pLoggingService->LogDebug("Duplicate points removed from polyline");
			hasInfo = true;
		}
		if (actualPointCount < 2)
		{
			pLoggingService->LogDebug("Polyline must have at least 2 vertices");
			hasInfo = true;
			return TopoDS_Wire();
		}

		//make the polygon
		TopoDS_Wire wire;
		builder.MakeWire(wire);
		for (int i = 2; i <= actualPointCount; i++)
		{
			BRepBuilderAPI_MakeEdge edgeMaker(TopoDS::Vertex(vertices.Value(i - 1)), TopoDS::Vertex(vertices.Value(i)));
			builder.Add(wire, edgeMaker.Edge());
		}
		wire.Closed(closed);

		return wire;

	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	return TopoDS_Wire();

}


//creates polyline removes  segments if they are >= tolerance in length, nb this method does not check for intersecting or notched lines
TopoDS_Wire NWireFactory::BuildPolyline3d(
	const TColgp_Array1OfPnt& points,
	double tolerance, bool& hasInfo)
{
	hasInfo = false;
	try
	{
		BRep_Builder builder;
		TopoDS_Wire wire;
		builder.MakeWire(wire);
		TopTools_SequenceOfShape vertices;
		const gp_Pnt& originalStart = points.First();
		const gp_Pnt& originalEnd = points.Last();
		double d = originalStart.Distance(originalEnd);
		bool closed = d <= tolerance; //check if the polyline is specified as closed (end repeats the start point)

		for (int i = 2; i <= points.Length(); i++)
		{
			const gp_Pnt& start = points.Value(i - 1);
			const gp_Pnt& end = points.Value(i);
			int vCount = vertices.Length();

			//	gp_Pnt startPoint = vCount == 0 ? startKP : BRep_Tool::Pnt(TopoDS::Vertex(vertices.Last()));
			double pointTolerance = tolerance + (vCount == 0 ? Precision::Confusion() : BRep_Tool::Tolerance(TopoDS::Vertex(vertices.Last())));

			gp_Vec edgeVec(start, end);
			double segLength = edgeVec.Magnitude();
			if (segLength < pointTolerance)
			{
				// We should used structured logging if we can.
				/*char message[128];
				sprintf_s(message, 128, "Polyline point ignored: (%f,%f,%f) is a duplicate within tolerance of previous point", end.X(), end.Y(), end.Z());
				pLoggingService->LogDebug(message);*/
				hasInfo = true;
				//adjust the position and precision of the previous vertex
				gp_Vec displacement = edgeVec.Divided(2);//get the vector to move to a point half way between the two
				gp_Pnt startTranslated = start.Translated(displacement);

				if (vertices.Length() == 0)
				{
					TopoDS_Vertex startVertex;
					builder.MakeVertex(startVertex, startTranslated, Precision::Confusion());
					vertices.Append(startVertex);
				}
				else
				{
					double toleranceOfFound = BRep_Tool::Tolerance(TopoDS::Vertex(vertices.Last()));
					double requiredTolerance = std::max(segLength + toleranceOfFound, segLength + Precision::Confusion());
					builder.UpdateVertex(TopoDS::Vertex(vertices.Last()), startTranslated, requiredTolerance); //make the found point a surrogate for both points
				}
				continue;
			}
			if (vertices.Length() == 0) //we have not added a point, put the first one in
			{
				TopoDS_Vertex startVertex;
				builder.MakeVertex(startVertex, start, Precision::Confusion());
				vertices.Append(startVertex);
			}
			TopoDS_Vertex endVertex;
			builder.MakeVertex(endVertex, end, Precision::Confusion());
			vertices.Append(endVertex);
		}

		for (int i = 2; i <= vertices.Length(); i++)
		{
			const TopoDS_Vertex& startVertex = TopoDS::Vertex(vertices.Value(i - 1));
			const TopoDS_Vertex& endVertex = TopoDS::Vertex(vertices.Value(i));
			gp_Pnt startPoint = BRep_Tool::Pnt(startVertex);
			gp_Pnt endPoint = BRep_Tool::Pnt(endVertex);
			BRepBuilderAPI_MakeEdge edgeMaker(startVertex, endVertex);
			builder.Add(wire, edgeMaker.Edge());
		}
		if (closed)
		{
			wire.Closed(true);
		}
		return wire;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	pLoggingService->LogWarning("Could not build polyline");
	return TopoDS_Wire();;

}
//turns the point list into a set of trimmed line segments, segments with a length less than tolerance are skipped
void NWireFactory::GetPolylineSegments3d(const TColgp_Array1OfPnt& points, double tolerance, TColGeom_SequenceOfBoundedCurve& curves)
{
	try
	{
		int pointCount = points.Length();
		int lastAddedStartPointIdx = 1;

		Handle(Geom_TrimmedCurve) lastAddedCurve;
		for (Standard_Integer i = 1; i < pointCount; i++)
		{
			const gp_Pnt& start = points.Value(lastAddedStartPointIdx);
			const gp_Pnt& end = points.Value(i + 1);

			double segLength = Abs(start.Distance(end));
			if (segLength > tolerance) //ignore very small segments
			{
				gp_Vec dir(start, end);
				gp_Lin line(start, dir);
				Handle(Geom_Line) hLine = new Geom_Line(line);
				lastAddedCurve = new Geom_TrimmedCurve(hLine, 0, dir.Magnitude());
				//move the lastIndex on
				lastAddedStartPointIdx = i + 1;
				curves.Append(lastAddedCurve);
			}
			else // we skip a segment because it is small lastPointIdx remains the same
			{
				if ((pointCount - 1) == i) //this is the last segment make sure end point does not move
				{
					const gp_Pnt& prevStart = curves.Last()->Value(0);
					gp_Vec dir(prevStart, end);
					gp_Lin line(prevStart, dir);
					Handle(Geom_Line) hLine = new Geom_Line(line);
					lastAddedCurve = new Geom_TrimmedCurve(hLine, 0, dir.Magnitude());
					curves.Remove(curves.Length()); //get rid of last one
					curves.Append(lastAddedCurve);
				}
			}
		}
		if (lastAddedStartPointIdx == 1) //we have failed to add anything
			Standard_Failure::Raise("The Polyline has no segments");
	}
	catch (const Standard_Failure& e)
	{
		pLoggingService->LogWarning("Could not build polyline");
		LogStandardFailure(e);
	}

}

//turns the point list into a set of trimmed line segments, segments with a length less than tolerance are skipped
void NWireFactory::GetPolylineSegments2d(const TColgp_Array1OfPnt2d& points, double tolerance, TColGeom2d_SequenceOfBoundedCurve& curves)
{

	try
	{
		int pointCount = points.Length();
		int lastAddedStartPointIdx = 1;

		Handle(Geom2d_TrimmedCurve) lastAddedCurve;
		for (Standard_Integer i = 1; i < pointCount; i++)
		{
			const gp_Pnt2d& start = points.Value(lastAddedStartPointIdx);
			const gp_Pnt2d& end = points.Value(i + 1);

			double segLength = Abs(start.Distance(end));
			if (segLength > tolerance) //ignore very small segments
			{
				gp_Vec2d dir(start, end);
				gp_Lin2d line(start, dir);
				Handle(Geom2d_Line) hLine = new Geom2d_Line(line);
				lastAddedCurve = new Geom2d_TrimmedCurve(hLine, 0, dir.Magnitude());
				//move the lastIndex on
				lastAddedStartPointIdx = i + 1;
				curves.Append(lastAddedCurve);
			}
			else // we skip a segment because it is small lastPointIdx remains the same
			{
				if ((pointCount - 1) == i) //this is the last segment make sure end point does not move
				{
					const gp_Pnt2d& prevStart = curves.Last()->Value(0);
					gp_Vec2d dir(prevStart, end);
					gp_Lin2d line(prevStart, dir);
					Handle(Geom2d_Line) hLine = new Geom2d_Line(line);
					lastAddedCurve = new Geom2d_TrimmedCurve(hLine, 0, dir.Magnitude());
					curves.Remove(curves.Length()); //get rid of last one
					curves.Append(lastAddedCurve);
				}
			}
		}
		if (lastAddedStartPointIdx == 1) //we have failed to add anything
			Standard_Failure::Raise("The Polyline has no segments");
	}
	catch (const Standard_Failure& e)
	{
		pLoggingService->LogWarning("Could not build polyline");
		LogStandardFailure(e);
	}



}
//Builds a wire a collection of segments and trims it
TopoDS_Wire NWireFactory::BuildDirectrixWire(const TopoDS_Wire& wire, double trimStart, double trimEnd, double tolerance, double gapSize)
{

	TopTools_SequenceOfShape edges;
	try
	{
		BRep_Builder builder;
		double parametricLength = 0;

		//std::ofstream myfile;
		//myfile.open("segments.txt");
		//for (auto& it = segments.cbegin(); it != segments.cend(); ++it)
		//{
		//	Handle(Geom_Curve) segment = *it;
		//	gp_Pnt segStartPoint = segment->Value(segment->FirstParameter()); //start and end of segment to add
		//	gp_Pnt segEndPoint = segment->Value(segment->LastParameter());

		//	myfile << segStartPoint.X() << "," << segStartPoint.Y() << "," << segStartPoint.Z() << "-->" << segEndPoint.X() << "," << segEndPoint.Y() << "," << segEndPoint.Z() << "Len:"<< segStartPoint.Distance(segEndPoint) <<std::endl;
		//}

		//myfile.close();

		for (auto wireExplorer = BRepTools_WireExplorer(wire); wireExplorer.More(); wireExplorer.Next())
		{
			const TopoDS_Edge& edge = wireExplorer.Current();
			Standard_Real start = 0, end = 0;
			TopLoc_Location loc;
			bool useExistingEdge = false;
			Handle(Geom_Curve) segment = BRep_Tool::Curve(edge, loc, start, end);

			double segLength = Abs(end - start);
			if (segLength < trimStart) //don't need this
				trimStart -= segLength;
			else
			{
				if (trimStart < segLength) //need a trimmed version of this segment
				{
					if (trimEnd <= parametricLength + segLength) //need no more after this
					{
						segment = new Geom_TrimmedCurve(segment, trimStart, trimEnd - parametricLength);
						parametricLength += (trimEnd - parametricLength);
						//stop concatenating
					}
					else //trimStart to end seg					
					{
						if (trimStart > 0)
							segment = new Geom_TrimmedCurve(segment, trimStart, end);
						parametricLength += (segment->LastParameter() - segment->FirstParameter());
					}
					trimStart = 0; //we have the first segment at 0
				}
				else //take the whole segment up to end trim
				{
					if (trimEnd <= parametricLength + segLength)
					{
						segment = new Geom_TrimmedCurve(segment, 0, trimEnd - parametricLength);
						parametricLength += (trimEnd - parametricLength);
						//stop concatenating
					}
					else //add the segment
					{
						useExistingEdge = true;
						parametricLength += (segment->LastParameter() - segment->FirstParameter());
					}
				}
				//make the segment an edge
				TopoDS_Edge anEdge;
				if (edges.Length() == 0) //just add the first one
				{
					if (useExistingEdge)
						anEdge = edge;
					else
					{
						BRepBuilderAPI_MakeEdge edgeMaker(segment);
						anEdge = edgeMaker.Edge();
					}
				}
				else //we need to add this segment to the start or end of the edges
				{

					const TopoDS_Edge& lastEdge = TopoDS::Edge(edges.Last());
					gp_Pnt lastEdgeEndPoint = BRep_Tool::Pnt(TopExp::LastVertex(lastEdge)); //the last point

					gp_Pnt segStartPoint = segment->Value(segment->FirstParameter()); //start and end of segment to add
					gp_Pnt segEndPoint = segment->Value(segment->LastParameter());

					double gap = segStartPoint.Distance(lastEdgeEndPoint);

					if (gap > gapSize)
					{
						Standard_Failure::Raise("Directrix Segments are not contiguous");
						//this segment is not connected to the previous one, make a new start vertex and create a discontinuous wire
					}
					else //connect to previous segment it is in tolerance
					{
						AdjustVertexTolerance(TopExp::LastVertex(lastEdge), lastEdgeEndPoint, segStartPoint, gap);
						TopoDS_Vertex segEndVertex;
						builder.MakeVertex(segEndVertex, segEndPoint, tolerance);
						if (useExistingEdge)
							anEdge = edge;
						else
						{
							BRepBuilderAPI_MakeEdge edgeMaker(segment, TopExp::LastVertex(lastEdge), segEndVertex);
							if (!edgeMaker.IsDone())
							{
								std::stringstream msg;
								msg << "Error building edge: Error Code: " << edgeMaker.Error();
								Standard_Failure::Raise(msg);
							}
							anEdge = edgeMaker.Edge();
						}
					}
				}
				if (!anEdge.IsNull())
					edges.Append(anEdge);
			}

		}
		if (edges.Size() == 0)
			Standard_Failure::Raise("Directrix has no Segments");
		TopoDS_Wire result;
		builder.MakeWire(result);
		for (const auto& edge : edges)
			builder.Add(result, edge);
		return result;
	}
	catch (const Standard_Failure& e)
	{
		pLoggingService->LogWarning("Could not build directrix");
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildTrimmedWire(const TopoDS_Wire& basisWire, double first, double last, bool sameSense, double tolerance, double radianFactor)
{
	return BuildTrimmedWire(basisWire, gp::Origin(), gp::Origin(), first, last, false, sameSense, tolerance, radianFactor);
}

TopoDS_Wire NWireFactory::BuildTrimmedWire(const TopoDS_Wire& basisWire, gp_Pnt p1, gp_Pnt p2, double u1, double u2, bool preferCartesian, bool sameSense, double tolerance, double radianFactor)
{
	try
	{


		BRepAdaptor_CompCurve cc(basisWire, Standard_True);
		GeomAbs_Shape continuity = cc.Continuity();
		int numIntervals = cc.NbIntervals(continuity);

		//calculate the start and end parameters
		double first;
		double last;
		if (preferCartesian)
		{
			if (!GetParameter(basisWire, p1, tolerance, first))
				Standard_Failure::Raise("Trim Point1 is not on the wire");
			if (!GetParameter(basisWire, p2, tolerance, last))
				Standard_Failure::Raise("Trim Point2 is not on the wire");
		}
		else
		{
			first = double::IsNaN(u1) ? 0 : u1;
			last = double::IsNaN(u2) ? cc.LastParameter() - cc.FirstParameter() : u2;
		}

		if (numIntervals == 1)
		{
			TopoDS_Edge edge; // the edge we are interested in
			Standard_Real uoe; // parameter U on the edge (not used)
			cc.Edge(last, edge, uoe);
			Standard_Real l, f; // the parameter range is returned in f and l
			Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
			Handle(Geom_TrimmedCurve) trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
			while (!trimmedCurve.IsNull()) //remove trims
			{
				curve = trimmedCurve->BasisCurve();
				trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(curve);
			}
			if (curve->IsPeriodic()) //stay in bounds for splines etc, keep orientation for periodics, 
			{
				l = last * radianFactor; //override any trims set, ensure in radians
				f = first * radianFactor;
			}
			else
			{
				f = System::Math::Max(f, first);
				l = System::Math::Min(l, last);
			}
			if (System::Math::Abs(f - l) > Precision::Confusion())
			{
				Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, f, l);
				BRepBuilderAPI_MakeWire wm;
				wm.Add(BRepBuilderAPI_MakeEdge(trimmed));
				return wm.Wire();
			}
			else
				return TopoDS_Wire(); //empty wire
		}
		else
		{
			BRepBuilderAPI_MakeWire wm;
			TColStd_Array1OfReal res(1, numIntervals + 1);
			cc.Intervals(res, GeomAbs_C0);
			for (Standard_Integer i = 1; i <= numIntervals; i++) //process all but the end interval point
			{
				Standard_Real fp = res.Value(i);
				Standard_Real lp = res.Value(i + 1);
				//if the first point is > lp then we do not want this edge at all
				if (first > lp)
					continue;
				//if the last point is < fp then we do not want it
				if (last < fp)
					continue;

				//we are going to need an edge
				TopoDS_Edge edge; // the edge we are interested in
				Standard_Real uoe; // parameter U on the edge (not used)
				cc.Edge(fp, edge, uoe);
				Standard_Real lEdge, fEdge; // the parameter range is returned in f and l
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, fEdge, lEdge);

				// if first is < lp and last < lp then we need to do both trims on this edge
				if (first > fp && first < lp && last < lp)
				{
					gp_Pnt pFirst = cc.Value(first);
					double uOnEdgeFirst;
					gp_Pnt pLast = cc.Value(last);
					double uOnEdgeLast;
					double maxTolerance = BRep_Tool::MaxTolerance(edge, TopAbs_VERTEX);
					GeomLib_Tool::Parameter(curve, pFirst, maxTolerance, uOnEdgeFirst);
					GeomLib_Tool::Parameter(curve, pLast, maxTolerance, uOnEdgeLast);
					if (System::Math::Abs(uOnEdgeFirst - uOnEdgeLast) > Precision::Confusion())
					{
						Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, uOnEdgeFirst, uOnEdgeLast);
						wm.Add(BRepBuilderAPI_MakeEdge(trimmed));
					}
				}
				// if first is < lp  then we need to trim to end of this edge unless first is zero or has already been used
				else if (first > 0 && first < lp)
				{
					gp_Pnt pFirst = cc.Value(first);
					double uOnEdgeFirst;
					double maxTolerance = BRep_Tool::MaxTolerance(edge, TopAbs_VERTEX);
					GeomLib_Tool::Parameter(curve, pFirst, maxTolerance, uOnEdgeFirst);
					if (System::Math::Abs(uOnEdgeFirst - lEdge) > Precision::Confusion())
					{
						Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, uOnEdgeFirst, lEdge);
						wm.Add(BRepBuilderAPI_MakeEdge(trimmed));
					}
					first = -1; //it has been done
				}
				//if last  < lp need to trim from beginning to last
				else if (last < lp)
				{
					//get the point required

					gp_Pnt pLast = cc.Value(last);
					double uOnEdgeLast;
					double maxTolerance = BRep_Tool::MaxTolerance(edge, TopAbs_VERTEX);
					GeomLib_Tool::Parameter(curve, pLast, maxTolerance, uOnEdgeLast);
					if (std::abs(uOnEdgeLast - fEdge) > Precision::Confusion())
					{
						Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, fEdge, uOnEdgeLast);
						wm.Add(BRepBuilderAPI_MakeEdge(trimmed));
					}
				}
				else //we want the whole edge
					wm.Add(edge);

				if (!wm.IsDone()) Standard_Failure::Raise("Error trimming wire, see logs");
			}
			return wm.Wire();
		}
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildWire(const TColGeom2d_SequenceOfBoundedCurve& segments, double tolerance, double gapSize)
{
	ShapeFix_Edge toleranceFixer;
	TopTools_SequenceOfShape edges;
	try
	{
		BRep_Builder builder;
		TopoDS_Wire wire;
		TopoDS_Edge anEdge;
		TopoDS_Edge theFirstEdge;
		TopoDS_Vertex theFirstVertex;
		gp_Pnt theFirstPoint;
		int nbSegments = segments.Length();
		int currentSegment = 0;
		bool isClosed = false;
		
		for (auto&& segment : segments)
		{
			bool tolerancesAdjusted = false;
			
			if (edges.Length() == 0) //just add the first one
			{
				BRepBuilderAPI_MakeEdge2d edgeMaker(segment);
				anEdge = edgeMaker.Edge();
				theFirstEdge = anEdge;
				theFirstVertex = TopExp::FirstVertex(anEdge);
				theFirstPoint = BRep_Tool::Pnt(theFirstVertex);
				currentSegment++;
			}
			else //we need to add this segment to the start of end of the edges
			{

				const TopoDS_Edge& lastEdge = TopoDS::Edge(edges.Last());
				gp_Pnt lastEdgeEndPoint = BRep_Tool::Pnt(TopExp::LastVertex(lastEdge)); //the last point

				gp_Pnt2d segStartPoint2d = segment->Value(segment->FirstParameter()); //start and end of segment to add
				gp_Pnt2d segEndPoint2d = segment->Value(segment->LastParameter());
				gp_Pnt segStartPoint(segStartPoint2d.X(), segStartPoint2d.Y(), 0);
				gp_Pnt segEndPoint(segEndPoint2d.X(), segEndPoint2d.Y(), 0);

				double gap = segStartPoint.Distance(lastEdgeEndPoint);

				if (gap > gapSize)
					Standard_Failure::Raise("Segments are not contiguous");
				currentSegment++;
				tolerancesAdjusted = AdjustVertexTolerance(TopExp::LastVertex(lastEdge), lastEdgeEndPoint, segStartPoint, gap);
				TopoDS_Vertex segEndVertex;
				if (currentSegment == nbSegments && theFirstPoint.Distance(segEndPoint) < gapSize) //its the last one
				{
					isClosed = true;
					double closingGap = segEndPoint.Distance(theFirstPoint);
					tolerancesAdjusted = AdjustVertexTolerance(TopExp::FirstVertex(TopoDS::Edge(edges.First())), theFirstPoint, segEndPoint, closingGap);
				}
				else
					builder.MakeVertex(segEndVertex, segEndPoint, tolerance);

				BRepBuilderAPI_MakeEdge2d edgeMaker(segment, TopExp::LastVertex(lastEdge), isClosed ? TopExp::FirstVertex(TopoDS::Edge(edges.First())) : segEndVertex);
				if (!edgeMaker.IsDone())
				{
					Standard_Real cf = segment->FirstParameter();
					Standard_Real cl = segment->LastParameter();
					gp_Pnt2d P1 = segment->Value(cf);
					gp_Pnt2d P2 = segment->Value(cl);
					double dist = P1.Distance(P2);
					std::stringstream msg;
					msg << "Error building edge: Error Code: " << edgeMaker.Error();
					Standard_Failure::Raise(msg);
				}
				anEdge = edgeMaker.Edge();
				if (tolerancesAdjusted) //fix up any edge issues arising
				{
					if (isClosed)
						toleranceFixer.FixVertexTolerance(theFirstEdge);
					else
						toleranceFixer.FixVertexTolerance(lastEdge);
					toleranceFixer.FixVertexTolerance(anEdge);
				}
			}
			bool ok = BRepLib::BuildCurve3d(anEdge, tolerance);
			if (ok)
				edges.Append(anEdge);
			else
				Standard_Failure::Raise("Could not create 3d PCurves in NWireFactory::BuildWire");
		}
		if (edges.Length() > 0)
		{
			//add the edges to the wire
			builder.MakeWire(wire);
			for (auto& it = edges.cbegin(); it != edges.cend(); ++it)
			{
				builder.Add(wire, *it);
			}
			if (isClosed) wire.Closed(true);
		}
		return wire;

	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildWire(const TColGeom_SequenceOfBoundedCurve& segments, double tolerance, double gapSize)
{
	ShapeFix_Edge toleranceFixer;
	TopTools_SequenceOfShape edges;
	try
	{
		BRep_Builder builder;
		TopoDS_Wire wire;
		TopoDS_Edge anEdge;
		TopoDS_Vertex theFirstVertex;
		TopoDS_Edge theFirstEdge;
		gp_Pnt theFirstPoint;
		int nbSegments = segments.Length();
		int currentSegment = 0;
		bool isClosed = false;
		bool lastSegmentIsPeriodic = false;
		bool currentSegmentIsPeriodic = false;
		Handle(Geom_Curve) lastBasisCurve;
		Handle(Geom_Curve) lastSegment;
		for (auto&& segIt : segments)
		{
			bool tolerancesAdjusted = false;
			Handle(Geom_Curve) segment = segIt;

			Handle(Geom_TrimmedCurve)trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(segment);
			Handle(Geom_Curve) basisCurve;
			while (!trimmedCurve.IsNull())
			{
				basisCurve = trimmedCurve->BasisCurve();
				trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(basisCurve);
			}
			if (!basisCurve.IsNull())
				currentSegmentIsPeriodic = basisCurve->IsPeriodic();
			else
				currentSegmentIsPeriodic = segment->IsPeriodic();
			if (edges.Length() == 0) //just add the first one
			{
				BRepBuilderAPI_MakeEdge edgeMaker(segment);
				anEdge = edgeMaker.Edge();
				theFirstVertex = TopExp::FirstVertex(anEdge);
				theFirstPoint = BRep_Tool::Pnt(theFirstVertex);
				currentSegment++;
				lastSegmentIsPeriodic = segment->IsPeriodic();
				lastSegment = segment;
				lastBasisCurve = basisCurve;
			}
			else //we need to add this segment to the start of end of the edges
			{



				gp_Pnt lastEdgeEndPoint = BRep_Tool::Pnt(TopExp::LastVertex(TopoDS::Edge(edges.Last()))); //the last point

				gp_Pnt segStartPoint = segment->Value(segment->FirstParameter()); //start and end of segment to add
				gp_Pnt segEndPoint = segment->Value(segment->LastParameter());

				double gap = segStartPoint.Distance(lastEdgeEndPoint);
				double gapend = segEndPoint.Distance(lastEdgeEndPoint);
				if (gap > gapSize)
					Standard_Failure::Raise("Segments are not contiguous");
				currentSegment++;
				if (currentSegmentIsPeriodic && !lastSegmentIsPeriodic) //change the coordinates of the last segments end point to be the same as the periodic curve, so that curves always fit
				{
					Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(lastBasisCurve);
					if (!line.IsNull())
					{
						gp_Pnt lastSegStarPoint = BRep_Tool::Pnt(TopExp::FirstVertex(TopoDS::Edge(edges.Last())));
						gp_Vec dir(lastSegStarPoint, segStartPoint);
						gp_Lin line(lastSegStarPoint, dir);
						Handle(Geom_Line) hLine = new Geom_Line(line);
						Handle(Geom_TrimmedCurve) newSegment = new Geom_TrimmedCurve(hLine, 0, dir.Magnitude());
						BRep_Builder b;
						b.UpdateVertex(TopExp::LastVertex(TopoDS::Edge(edges.Last())), segStartPoint, tolerance);
						BRepBuilderAPI_MakeEdge edgeMaker(newSegment, TopExp::FirstVertex(TopoDS::Edge(edges.Last())), TopExp::LastVertex(TopoDS::Edge(edges.Last())));
						auto newEdge = edgeMaker.Edge();
						edges.Remove(edges.Size());
						edges.Append(newEdge);
						lastEdgeEndPoint = segStartPoint;
						gap = 0;
					}

				}
				if (lastSegmentIsPeriodic && !currentSegmentIsPeriodic) //change the coordinates of the last segments end point to be the same as the periodic curve, so that curves always fit
				{

					//if it's a line rebuild the geometry
					Handle(Geom_Line) line = Handle(Geom_Line)::DownCast(basisCurve);
					if (!line.IsNull())
					{
						segStartPoint = lastEdgeEndPoint;
						gp_Vec dir(segStartPoint, segEndPoint);
						gp_Lin line(segStartPoint, dir);
						Handle(Geom_Line) hLine = new Geom_Line(line);
						segment = new Geom_TrimmedCurve(hLine, 0, dir.Magnitude());

						gap = 0;
					}
				}
				tolerancesAdjusted = AdjustVertexTolerance(TopExp::LastVertex(TopoDS::Edge(edges.Last())), lastEdgeEndPoint, segStartPoint, gap);
				TopoDS_Vertex segEndVertex;
				if (currentSegment == nbSegments && theFirstPoint.Distance(segEndPoint) < gapSize) //its the last one
				{
					isClosed = true;
					double closingGap = segEndPoint.Distance(theFirstPoint);
					tolerancesAdjusted = AdjustVertexTolerance(TopExp::FirstVertex(TopoDS::Edge(edges.First())), theFirstPoint, segEndPoint, closingGap);
				}
				else
					builder.MakeVertex(segEndVertex, segEndPoint, tolerance);

				BRepBuilderAPI_MakeEdge edgeMaker(segment, TopExp::LastVertex(TopoDS::Edge(edges.Last())), isClosed ? TopExp::FirstVertex(TopoDS::Edge(edges.First())) : segEndVertex);
				if (!edgeMaker.IsDone())
				{
					Standard_Real cf = segment->FirstParameter();
					Standard_Real cl = segment->LastParameter();
					gp_Pnt P1 = segment->Value(cf);
					gp_Pnt P2 = segment->Value(cl);
					double dist = P1.Distance(P2);
					std::stringstream msg;
					msg << "Error building edge: Error Code: " << edgeMaker.Error();
					Standard_Failure::Raise(msg);
				}
				anEdge = edgeMaker.Edge();
				lastBasisCurve = basisCurve;
				lastSegmentIsPeriodic = currentSegmentIsPeriodic;
				lastSegment = segment;
			}
			edges.Append(anEdge);
		}
		if (edges.Length() > 0)
		{
			//add the edges to the wire
			builder.MakeWire(wire);
			for (auto&& edge : edges)
			{
				toleranceFixer.FixVertexTolerance(TopoDS::Edge(edge));
				builder.Add(wire, edge);
			}
			if (isClosed) wire.Closed(true);

		}
		return wire;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}

bool NWireFactory::AdjustVertexTolerance(TopoDS_Vertex& vertexToJoinTo, gp_Pnt pointToJoinTo, gp_Pnt pointToJoin, double gap)
{

	double tolE = BRep_Tool::Tolerance(vertexToJoinTo);
	double maxtol = .5 * (tolE + gap), cW = 1, cE = 0;
	BRep_Builder b;
	if (maxtol > tolE)
	{
		cW = (maxtol - tolE) / gap;
		cE = 1. - cW;
		gp_Pnt PC(cW * pointToJoinTo.X() + cE * pointToJoin.X(), cW * pointToJoinTo.Y() + cE * pointToJoin.Y(), cW * pointToJoinTo.Z() + cE * pointToJoin.Z());

		b.UpdateVertex(vertexToJoinTo, PC, maxtol);
		return true;
	}
	else //just set the tolerance to the larger of the distance or the existing tolerance
	{
		if (gap > tolE)
		{
			b.UpdateVertex(vertexToJoinTo, pointToJoinTo, gap);
			return true;
		}
	}
	return false;
}

TopoDS_Wire NWireFactory::BuildRectangleProfileDef(double xDim, double yDim)
{
	try
	{
		double xOff = xDim / 2;
		double yOff = yDim / 2;
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
		return BRepBuilderAPI_MakeWire(e1, e2, e3, e4);
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Could not build rectangle profile def");
	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildCircleProfileDef(double radius, const gp_Ax22d& position)
{
	try
	{

		gp_Ax2 ax2(gp_Pnt(position.Location().X(), position.Location().Y(), 0), gp::DZ(), gp_Dir(position.XDirection().X(), position.XDirection().Y(), 0));
		Handle(Geom_Circle) hCirc = GC_MakeCircle(ax2, radius);

		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hCirc);
		return BRepBuilderAPI_MakeWire(edge);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildOffset(TopoDS_Wire basisWire, double distance)
{
	try
	{
		BRepOffsetAPI_MakeOffset offsetMaker(basisWire);
		offsetMaker.Perform(distance);
		if (!offsetMaker.IsDone() || offsetMaker.Shape().ShapeType() != TopAbs_WIRE)
			Standard_Failure::Raise("Error building offset curve as a wire");
		return TopoDS::Wire(offsetMaker.Shape());
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return TopoDS_Wire();
}

TopoDS_Wire NWireFactory::BuildOffset(TopoDS_Wire basisWire, double distance, gp_Dir dir)
{
	return TopoDS_Wire();

}

TopoDS_Wire NWireFactory::Fillet(const TopoDS_Wire& wire, double filletRadius, double tolerance)
{
	try
	{
		if (wire.IsNull()) return wire;

		Standard_Integer nbEdges = wire.NbChildren();

		//get an array of all the edges
		TopTools_Array1OfShape edges(1, nbEdges);
		TopTools_Array1OfShape vertices(1, nbEdges);
		TopTools_Array1OfShape filleted(1, nbEdges * 2);
		Standard_Integer nb = 0;
		for (BRepTools_WireExplorer edgeExp(wire); edgeExp.More(); edgeExp.Next())
		{
			nb++;
			edges(nb) = TopoDS::Edge(edgeExp.Current());
			vertices(nb) = TopoDS::Vertex(edgeExp.CurrentVertex());
		}
		//need to do each pair to ensure they are on face
		int totalEdges = 1;
		for (int i = 1; i < nbEdges; i++)
		{
			BRepBuilderAPI_MakeWire filletWireMaker;
			filletWireMaker.Add(TopoDS::Edge(edges(i)));
			filletWireMaker.Add(TopoDS::Edge(edges(i + 1)));
			BRepBuilderAPI_MakeFace faceMaker(filletWireMaker.Wire());
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
			filleter.AddFillet(TopoDS::Vertex(vertices(i + 1)), filletRadius);
			filleter.Build();
			if (filleter.IsDone() && filleter.NbFillet() > 0)
			{
				const TopTools_SequenceOfShape& fillets = filleter.FilletEdges();
				filleted(2 * i - 1) = filleter.DescendantEdge(TopoDS::Edge(edges(i)));
				edges(i) = filleted(2 * i - 1);
				filleted(2 * i) = fillets(1);
				filleted(2 * i + 1) = filleter.DescendantEdge(TopoDS::Edge(edges(i + 1)));
				edges(i + 1) = filleted(2 * i + 1);
				totalEdges += 2;

			}
			else //no fillet happened just store existing
			{
				pLoggingService->LogInformation("Failed to fillet wire edge");
				filleted(2 * i - 1) = edges(i);
				filleted(2 * i) = edges(i + 1);
				totalEdges++;
			}
		}
		if (IsClosed(wire, tolerance) && nbEdges > 1)
		{
			BRepBuilderAPI_MakeWire filletWireMaker;
			filletWireMaker.Add(TopoDS::Edge(edges(1)));
			filletWireMaker.Add(TopoDS::Edge(edges(nbEdges)));
			BRepBuilderAPI_MakeFace faceMaker(filletWireMaker.Wire());
			BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
			filleter.AddFillet(TopoDS::Vertex(vertices(1)), filletRadius);
			filleter.Build();
			if (filleter.IsDone() && filleter.NbFillet() > 0)
			{
				const TopTools_SequenceOfShape& fillets = filleter.FilletEdges();
				filleted(2 * nbEdges - 1) = filleter.DescendantEdge(TopoDS::Edge(edges(nbEdges)));
				filleted(2 * nbEdges) = fillets(1);
				filleted(1) = filleter.DescendantEdge(TopoDS::Edge(edges(1)));
				totalEdges++;
			}
			else
				pLoggingService->LogInformation("Failed to close fillet wire edge");

		}
		BRepBuilderAPI_MakeWire wireMaker;
		for (int i = 1; i <= totalEdges; i++)
		{
			if (!TopoDS::Edge(filleted(i)).IsNull())
				wireMaker.Add(TopoDS::Edge(filleted(i)));
		}

		if (!wireMaker.IsDone())
			Standard_Failure::Raise("Failed to fillet wire");
		else
			return wireMaker.Wire();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return TopoDS_Wire();
}


bool NWireFactory::GetParameter(const TopoDS_Wire& wire, gp_Pnt pnt, double tolerance, double& val)
{
	try
	{
		double paramOffset = 0;
		for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
		{
			Standard_Real u = 0;
			Standard_Real fpar = 0, lpar = 0;
			TopLoc_Location aLoc;
			const TopoDS_Edge& edge = TopoDS::Edge(exp.Current());
			Handle(Geom_Curve) aCurve = BRep_Tool::Curve(edge, aLoc, fpar, lpar);
			if (GeomLib_Tool::Parameter(aCurve, pnt, tolerance, u))
			{
				val = paramOffset + u;
				return true;
			}
			GProp_GProps gProps;
			BRepGProp::LinearProperties(edge, gProps);
			paramOffset += gProps.Mass();
		}
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return false;
}

bool NWireFactory::GetNormal(const TopoDS_Wire& wire, gp_Vec& normal)
{
	if (wire.IsNull()) return false; //no wire, no normal
	TColgp_SequenceOfPnt points;
	for (BRepTools_WireExplorer exEdge(wire); exEdge.More(); exEdge.Next())
	{
		TopoDS_Edge edge = TopoDS::Edge(exEdge.Current());
		TopoDS_Vertex sv = TopExp::FirstVertex(edge, Standard_True);
		gp_Pnt pnt = BRep_Tool::Pnt(sv);
		points.Append(pnt);
	}
	int total = points.Length();
	if (total == 0) return false; //no points, no normal
	double x = 0, y = 0, z = 0;
	gp_Pnt previous;
	int count = 0;

	for (int i = 0; i <= total; i++)
	{
		gp_Pnt current = i < total ? points.Value(i + 1) : points.Value(1);
		if (count > 0)
		{
			double xn = previous.X();
			double yn = previous.Y();
			double zn = previous.Z();
			double xn1 = current.X();
			double yn1 = current.Y();
			double zn1 = current.Z();
			x += (yn - yn1) * (zn + zn1);
			y += (xn + xn1) * (zn - zn1);
			z += (xn - xn1) * (yn + yn1);
		}
		previous = current;
		count++;
	}
	gp_Vec v(x, y, z);
	if (v.Magnitude() >= gp::Resolution())
	{
		normal = v.Normalized();
		return true;
	}
	else //it is not valid
	{

		return false;
	}
}

double NWireFactory::Area(const TopoDS_Wire& wire)
{
	return ShapeAnalysis::ContourArea(wire);
}
