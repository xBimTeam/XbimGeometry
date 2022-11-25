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

TopoDS_Wire NWireFactory::BuildPolyline2d(const TColgp_Array1OfPnt2d& points, double tolerance)
{
	//we need to ensure that no points are technically duplicate, i.e. < tolerance of the model apart
	//the method of removing duplicates that are tolerance away from each other is flawed when the polyline is used
	//in a shared vertex context such as a brep definition, using cell filters and controlling precision is better
	try
	{
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
		for (auto& it = pointSeq.cbegin(); it != pointSeq.cend(); ++it)
		{
			pCount--;
			gp_XY pnt = it->myPnt2d;
			gp_XY pMin = gp_XY(pnt.X() - tolerance, pnt.Y() - tolerance);
			gp_XY pMax = gp_XY(pnt.X() + tolerance, pnt.Y() + tolerance);
			anInspector.SetPoint(pnt);
			theCells.Inspect(pMin, pMax, anInspector);
			int aResID = anInspector.GetCoincidentPoint();

			if (aResID <= 0) //its not in the vertex content we need to create it
			{
				BRepMesh_Vertex keyedVertex(pnt, it->myID, BRepMesh_DegreeOfFreedom::BRepMesh_OnCurve);
				myLastId = anInspector.Add(keyedVertex);
				theCells.Add(myLastId, pMin, pMax);
				vertices.Append(it->CreateTopoVertex());
			}
			else //we have a coincidental vertex from the context
			{
				//adjust its tolerance and position so that the shared vertex is a surrogate for both regardless 
				const BRepMesh_Vertex& keyedPnt = anInspector.GetVertex(aResID);
				gp_XY foundPoint = keyedPnt.Coord();
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
					char message[128];
					sprintf_s(message, 128, "Polyline point ignored: #%d is a duplicate of #%d", it->myID, keyedPnt.Location3d());
					pLoggingService->LogInformation(message);
					continue;
				}
				else if (pCount != 0) //we are adding a point we already have connnected and we are not on the last point
				{
					if (!warnedOfSelfIntersection)
					{
						pLoggingService->LogInformation("Self intersecting polyline");
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
			pLoggingService->LogInformation("Duplicate points removed from polyline");
		if (actualPointCount < 2)
		{
			pLoggingService->LogInformation("Polyline must have at least 2 vertices");
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


//trims the polyline to the parameters, if they are >=0, nb this method does not check for intersecting or notched lines
TopoDS_Wire NWireFactory::BuildPolyline3d(
	const TColgp_Array1OfPnt& points, double startParam, double endParam,
	double tolerance)
{
	//3d polylines don't necessarily sit on a surface, they may in the case of an brep face, 
	//in these cases we have a surface and the BuildPolyline method with a surface should be called
	//no checks ar eperformed here apart from stopping zero length edges
	try
	{
		int id = 0;
		NCollection_Vector<KeyedPnt> pointSeq;
		for (auto&& point : points)
		{
			pointSeq.Append(KeyedPnt(point.XYZ(), ++id));
		}

		BRep_Builder builder;

		//make the polygon

		TopoDS_Wire wire;
		builder.MakeWire(wire);
		double offset = 0;
		TopTools_SequenceOfShape vertices;
		gp_Pnt start = pointSeq.cbegin()->myPnt;
		gp_Pnt end = pointSeq.Last().myPnt;
		double d = start.Distance(end);
		bool closed = d <= tolerance; //check if the polyline is specified as closed (end repeats the start point)

		for (int i = 1; i < pointSeq.Length(); i++)
		{
			const KeyedPnt& startKP = pointSeq.Value(i - 1);
			const KeyedPnt& endKP = pointSeq.Value(i);
			int vCount = vertices.Length();

			gp_Pnt startPoint = vCount == 0 ? startKP.myPnt : BRep_Tool::Pnt(TopoDS::Vertex(vertices.Last()));
			double pointTolerance = tolerance + (vCount == 0 ? Precision::Confusion() : BRep_Tool::Tolerance(TopoDS::Vertex(vertices.Last())));
			gp_Pnt endPoint = endKP.myPnt;
			gp_Vec edgeVec(startPoint, endPoint);
			double segLength = edgeVec.Magnitude();
			if (segLength < pointTolerance)
			{
				char message[128];
				sprintf_s(message, 128, "Polyline point ignored: #%d is a duplicate of #%d", endKP.myID, startKP.myID);
				pLoggingService->LogInformation(message);
				//adjust the position and precision of the previous vertex
				gp_Vec displacement = edgeVec.Divided(2);//get the vector to move to a point half way between the two
				startPoint.Translate(displacement);

				if (vertices.Length() == 0)
				{
					TopoDS_Vertex startVertex;
					builder.MakeVertex(startVertex, startPoint, Precision::Confusion());
					vertices.Append(startVertex);
				}
				else
				{
					double toleranceOfFound = BRep_Tool::Tolerance(TopoDS::Vertex(vertices.Last()));
					double requiredTolerance = std::max(segLength + toleranceOfFound, segLength + Precision::Confusion());
					builder.UpdateVertex(TopoDS::Vertex(vertices.Last()), startPoint, requiredTolerance); //make the found point a surrogate for both points
				}
				continue;
			}

			if (startParam >= 0) //we want to clip, adjust the vertices if necessary
			{
				if (startParam >= offset && startParam < offset + segLength) //trim this edge its the first one, will only enter the first time
				{
					if (vertices.Length() == 0) //only add the start in once, if the length is > 0 then we have a small first segment and a point waiting to use
					{
						edgeVec.Normalize();
						startPoint.Translate(edgeVec * (startParam - offset)); //move the start point	
						TopoDS_Vertex startVertex;
						builder.MakeVertex(startVertex, startPoint, Precision::Confusion());
						vertices.Append(startVertex);
					}
					//check if it is also the last one
					if (endParam > 0 && endParam <= offset + segLength)
					{
						//trim this edge to start and end and give in
						endPoint = startPoint.Translated(edgeVec * (endParam - offset));
						TopoDS_Vertex endVertex;
						builder.MakeVertex(endVertex, endPoint, Precision::Confusion());
						vertices.Append(endVertex);
						break; //all done
					}
					startParam = -1; //don't look for anymore start edges
					TopoDS_Vertex endVertex;
					builder.MakeVertex(endVertex, endPoint, Precision::Confusion());
					vertices.Append(endVertex);
				}
			}
			else  //(startParam < 0) we want this
			{
				if (vertices.Length() == 0) //we have not added a point, put the first one in
				{
					TopoDS_Vertex startVertex;
					builder.MakeVertex(startVertex, startPoint, Precision::Confusion());
					vertices.Append(startVertex);
				}
				if (endParam > offset && endParam < (offset + segLength)) //need to trim its the last seg
				{
					edgeVec.Normalize();
					endPoint = startPoint.Translated(edgeVec * (endParam - offset));
					TopoDS_Vertex endVertex;
					builder.MakeVertex(endVertex, endPoint, Precision::Confusion());
					vertices.Append(endVertex);

					break; //don't do anymore
				}
				else
				{
					TopoDS_Vertex endVertex;
					builder.MakeVertex(endVertex, endPoint, Precision::Confusion());
					vertices.Append(endVertex);
				}
			}
			offset += segLength;
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
			int actualPointCount = vertices.Size();
			BRepBuilderAPI_MakeEdge edgeMaker(TopoDS::Vertex(vertices.Value(actualPointCount)), TopoDS::Vertex(vertices.Value(1)));
			builder.Add(wire, edgeMaker.Edge());
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
		TopoDS_Wire wire;
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
		;
		for (auto wireExplorer = BRepTools_WireExplorer(wire); wireExplorer.More(); wireExplorer.Next())
		{
			TopoDS_Edge edge = wireExplorer.Current();
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
							segment = new Geom_TrimmedCurve(segment, trimStart, segment->LastParameter());
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
		builder.MakeWire(wire);
		for (auto&& edge : edges)
			builder.Add(wire, edge);
		return wire;
	}
	catch (const Standard_Failure& e)
	{
		pLoggingService->LogWarning("Could not build directrix");
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}


TopoDS_Wire NWireFactory::BuildTrimmedWire(const TopoDS_Wire& basisWire, gp_Pnt p1, gp_Pnt p2, double u1, double u2, bool preferCartesian, bool sameSense, double tolerance)
{
	try
	{

		//BRepAdaptor_CompCurve cc(basisWire, Standard_True, first, last, tolerance); TODO: test if we can trim here and remove complexity
		BRepAdaptor_CompCurve cc(basisWire, Standard_True);
		GeomAbs_Shape continuity = cc.Continuity();
		int numIntervals = cc.NbIntervals(continuity);

		//calcualte the start and end parameters
		double first = u1, last = u2;
		if (preferCartesian)
		{
			if (!GetParameter(basisWire, p1, tolerance, first))
				Standard_Failure::Raise("Trim Point1 is not on the wire");
			if (!GetParameter(basisWire, p2, tolerance, last))
				Standard_Failure::Raise("Trim Point2 is not on the wire");
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
				l = f + last;
				f = f + first;
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
	TopTools_SequenceOfShape edges;
	try
	{
		BRep_Builder builder;
		TopoDS_Wire wire;
		TopoDS_Edge anEdge;

		//std::ofstream myfile;
		//myfile.open("segments.txt");
		//for (auto& it = segments.cbegin(); it != segments.cend(); ++it)
		//{
		//	Handle(Geom2d_Curve) segment = *it;
		//	gp_Pnt2d segStartPoint2d = segment->Value(segment->FirstParameter()); //start and end of segment to add
		//	gp_Pnt2d segEndPoint2d = segment->Value(segment->LastParameter());
		//	
		//	myfile << segStartPoint2d.X() << "," << segStartPoint2d.Y() << "-->" << segEndPoint2d.X() << "," << segEndPoint2d.Y() << std::endl;
		//}
		//
		//myfile.close();
		for (auto&& segIt : segments)
		{
			Handle(Geom2d_Curve) segment = segIt;

			if (edges.Length() == 0) //just add the first one
			{
				BRepBuilderAPI_MakeEdge2d edgeMaker(segment);
				anEdge = edgeMaker.Edge();
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
				AdjustVertexTolerance(TopExp::LastVertex(lastEdge), lastEdgeEndPoint, segStartPoint, gap);
				TopoDS_Vertex segEndVertex;
				builder.MakeVertex(segEndVertex, segEndPoint, tolerance);

				BRepBuilderAPI_MakeEdge2d edgeMaker(segment, TopExp::LastVertex(lastEdge), segEndVertex);
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
			}
			bool ok = BRepLib::BuildCurve3d(anEdge, tolerance);
			if (ok)
				edges.Append(anEdge);
			else
				Standard_Failure::Raise("Could not create 3d PCurves in NWireFactory::BuildWire");
		}
		//add the edges to the wire
		builder.MakeWire(wire);
		for (auto& it = edges.cbegin(); it != edges.cend(); ++it)
		{
			builder.Add(wire, *it);
		}
		double closingGap = segments.First()->Value(segments.First()->FirstParameter()).Distance(segments.Last()->Value(segments.First()->LastParameter()));
		if (closingGap < gapSize)
			wire.Closed(true);
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
	TopTools_SequenceOfShape edges;
	try
	{
		BRep_Builder builder;
		TopoDS_Wire wire;
		TopoDS_Edge anEdge;

		for (auto&& segIt : segments)
		{
			Handle(Geom_Curve) segment = segIt;

			if (edges.Length() == 0) //just add the first one
			{
				BRepBuilderAPI_MakeEdge edgeMaker(segment);
				anEdge = edgeMaker.Edge();
			}
			else //we need to add this segment to the start of end of the edges
			{

				const TopoDS_Edge& lastEdge = TopoDS::Edge(edges.Last());
				gp_Pnt lastEdgeEndPoint = BRep_Tool::Pnt(TopExp::LastVertex(lastEdge)); //the last point

				gp_Pnt segStartPoint = segment->Value(segment->FirstParameter()); //start and end of segment to add
				gp_Pnt segEndPoint = segment->Value(segment->LastParameter());

				double gap = segStartPoint.Distance(lastEdgeEndPoint);

				if (gap > gapSize)
					Standard_Failure::Raise("Segments are not contiguous");
				AdjustVertexTolerance(TopExp::LastVertex(lastEdge), lastEdgeEndPoint, segStartPoint, gap);
				TopoDS_Vertex segEndVertex;
				builder.MakeVertex(segEndVertex, segEndPoint, tolerance);

				BRepBuilderAPI_MakeEdge edgeMaker(segment, TopExp::LastVertex(lastEdge), segEndVertex);
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
			}
			/*bool ok = BRepLib::BuildCurve3d(anEdge, tolerance);
			if (ok)*/
			edges.Append(anEdge);
			/*else
				Standard_Failure::Raise("Could not create 3d PCurves in NWireFactory::BuildWire");*/
		}
		//add the edges to the wire
		builder.MakeWire(wire);
		for (auto& it = edges.cbegin(); it != edges.cend(); ++it)
		{
			builder.Add(wire, *it);
		}
		double closingGap = segments.First()->Value(segments.First()->FirstParameter()).Distance(segments.Last()->Value(segments.First()->LastParameter()));
		if (closingGap < gapSize)
			wire.Closed(true);
		return wire;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}

	return TopoDS_Wire();
}

void NWireFactory::AdjustVertexTolerance(TopoDS_Vertex& vertexToJoinTo, gp_Pnt pointToJoinTo, gp_Pnt pointToJoin, double gap)
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
	}
	else //just set the tolerance to the larger of the distance or the exisiting tolerance
	{
		if (gap > tolE)
			b.UpdateVertex(vertexToJoinTo, pointToJoinTo, gap);
	}
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
//
//TopoDS_Wire NWireFactory::BuildDirectrix(TColGeom_SequenceOfCurve& segments, double trimStart, double trimEnd, double tolerance)
//{
//	TopTools_SequenceOfShape edges;
//	try
//	{
//		BRep_Builder builder;
//		double parametricLength = 0;
//		TopoDS_Wire wire;
//		for (auto&& segment : segments)
//		{
//			double segLength = Abs(segment->LastParameter() - segment->FirstParameter());
//			if (segLength < trimStart) //don't need this
//				trimStart -= segLength;
//			else
//			{
//				if (trimStart < segLength) //need a trimmed version of this segment
//				{
//					if (trimEnd <= parametricLength + segLength) //need no more after this
//					{
//						segment = new Geom_TrimmedCurve(segment, trimStart, trimEnd - parametricLength);
//						parametricLength += (trimEnd - parametricLength);
//						//stop concatenating
//					}
//					else //trimStart to end seg					
//					{
//						if (trimStart > 0)
//							segment = new Geom_TrimmedCurve(segment, trimStart, segment->LastParameter());
//						parametricLength += (segment->LastParameter() - segment->FirstParameter());
//					}
//					trimStart = 0; //we have the first segment at 0
//				}
//				else //take the whole segment up to end trim
//				{
//					if (trimEnd <= parametricLength + segLength)
//					{
//						segment = new Geom_TrimmedCurve(segment, 0, trimEnd - parametricLength);
//						parametricLength += (trimEnd - parametricLength);
//						//stop concatenating
//					}
//					else //add the segment
//					{
//						parametricLength += (segment->LastParameter() - segment->FirstParameter());
//					}
//				}
//				//make the segment an edge
//
//				if (edges.Length() == 0) //just add the first one
//				{
//					BRepBuilderAPI_MakeEdge edgeMaker(segment);
//					TopoDS_Edge edgeToAdd = edgeMaker.Edge();
//					edges.Append(edgeToAdd);
//				}
//				else //we need to add this segment ot the start of end of the edges
//				{
//					TopoDS_Edge lastEdge = TopoDS::Edge(edges.Last());
//					TopoDS_Vertex vLast = TopExp::LastVertex(lastEdge); //get the vertex at the end of the wire
//					gp_Pnt lastPoint = BRep_Tool::Pnt(vLast); //the last point
//
//					gp_Pnt segStartPoint = segment->Value(segment->FirstParameter()); //start and end of segment to add
//					gp_Pnt segEndPoint = segment->Value(segment->LastParameter());
//
//					double lastGap = lastPoint.Distance(segStartPoint);
//					
//					if (lastGap > tolerance)
//						Standard_Failure::Raise("Segments are not contiguous");
//					AdjustVertexTolerance(vLast, lastPoint, segStartPoint, lastGap);
//					TopoDS_Vertex segEndVertex;
//					builder.MakeVertex(segEndVertex, segEndPoint, Precision::Confusion());
//					BRepBuilderAPI_MakeEdge edgeMaker(segment, vLast, segEndVertex);
//					edges.Append(edgeMaker.Edge());
//				}
//			}
//		}
//		builder.MakeWire(wire);
//		for (auto&& edge: edges)
//			builder.Add(wire, edge);
//		return wire;
//	}
//	catch (const Standard_Failure& e)
//	{
//		pLoggingService->LogWarning("Could not build directrix");
//		LogStandardFailure(e);
//	}
//	
//	return TopoDS_Wire();
//}

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
