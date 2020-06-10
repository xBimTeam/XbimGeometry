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

TopoDS_Wire NWireFactory::BuildPolyline2d(
	const NCollection_Vector<KeyedPnt2d>& pointSeq,
	double tolerance, bool buildRaw)
{
	//we need to ensure that no points are technically duplicate, i.e. < tolerance of the model apart
	//the method of removing duplicates that are tolerance away from each other is flawed when the polyline is used
	//in a shared vertex context such as a brep definition, using cell filters and controlling precision is better
	try
	{
		BRep_Builder builder;
		NCollection_CellFilter<BRepMesh_VertexInspector> theCells(tolerance, new NCollection_IncAllocator);
		gp_Pnt2d start = pointSeq.begin()->myPnt2d;
		gp_Pnt2d end = pointSeq.end()->myPnt2d;
		bool closed = start.Distance(end) <= tolerance; //check if the polyline is specified as closed (end repeats the start point)

		int myLastId = -1;
		bool warnedOfSelfIntersection = false;
		BRepMesh_VertexInspector anInspector(new NCollection_IncAllocator);
		anInspector.SetTolerance(tolerance);

		TopTools_SequenceOfShape vertices;
		int pCount = pointSeq.Size();
		for (auto it = pointSeq.cbegin(); it != pointSeq.cend(); ++it)
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
				else if (aResID != 1) //we are adding a point we alread have connnected unless it is the first point it will make an illegal topological wire
				{
					if (!warnedOfSelfIntersection)
					{
						pLoggingService->LogInformation("Self intersecting polyline");
						warnedOfSelfIntersection = true; //just do it once
					}
					vertices.Append(kv);
				}
			}

		}
		//The inspector now has our final vertex list
		int desiredPointCount = closed ? pointSeq.Length() - 1 : pointSeq.Length();
		int actualPointCount = vertices.Size();
		if (actualPointCount < desiredPointCount) //we have removed duplicate points
			pLoggingService->LogInformation("Duplicate points removed from polyline");
		if (actualPointCount < 2)
		{
			pLoggingService->LogInformation("Polyline must have at least 2 vertices");
			return _emptyWire;
		}
		if (closed && actualPointCount < 3)
		{
			pLoggingService->LogInformation("Cannot close a polyline with less than 3 vertices");
			closed = false;
		}
		//make the polygon

		TopoDS_Wire wire;
		builder.MakeWire(wire);
		for (int i = 2; i <= actualPointCount; i++)
		{
			BRepBuilderAPI_MakeEdge edgeMaker(TopoDS::Vertex(vertices.Value(i - 1)), TopoDS::Vertex(vertices.Value(i)));
			builder.Add(wire, edgeMaker.Edge());
		}
		if (closed)
		{
			BRepBuilderAPI_MakeEdge edgeMaker(TopoDS::Vertex(vertices.Value(actualPointCount)), TopoDS::Vertex(vertices.Value(1)));
			builder.Add(wire, edgeMaker.Edge());
		}

		TopoDS_Face face = BRepBuilderAPI_MakeFace(gp_Pln(gp::Origin(), gp::DZ()));


		builder.Add(face, wire);
		ShapeAnalysis_Wire wireAnalyser(wire, face, tolerance);

		bool fixSelfIntersection = wireAnalyser.CheckSelfIntersection(); //this finds self intersection even if the edges are not adjacent, the fixer only fixes adjacent

		if (fixSelfIntersection || warnedOfSelfIntersection)
		{
			ShapeFix_Wire wireFixer(wire, face, tolerance);
			Handle(ShapeBuild_ReShape) rebuildShape = new ShapeBuild_ReShape(); //need these to ensure changes happen
			wireFixer.SetContext(rebuildShape);
			if (fixSelfIntersection) wireFixer.FixSelfIntersection();
			if (warnedOfSelfIntersection) wireFixer.FixNotchedEdges(); //this will fix lines that are colinear with the next one
			pLoggingService->LogInformation("Self-Intersecting polyline fixed");
			return wireFixer.Wire();
		}
		else
			return wire;

	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}

	pLoggingService->LogWarning("Could not build polyline");
	return _emptyWire;

}

//trims the polyline to the parameters, if they are >=0, nb this method does not check for intersecting or notched lines
TopoDS_Wire NWireFactory::BuildPolyline(
	const NCollection_Vector<KeyedPnt>& pointSeq, double startParam, double endParam,
	double tolerance)
{
	//3d polylines don't necessarily sit on a surface, they may in the case of an brep face, 
	//in these cases we have a surface and the BuildPolyline method with a surface should be called
	//no checks ar eperformed here apart from stopping zero length edges
	try
	{
		BRep_Builder builder;

		//make the polygon

		TopoDS_Wire wire;
		builder.MakeWire(wire);
		double offset = 0;
		TopTools_SequenceOfShape vertices;

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

		return wire;
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}

	pLoggingService->LogWarning("Could not build polyline");
	return _emptyWire;

}
//turns the point list into a set of trimmed line segments, segments with a length less than tolerance are skipped
void NWireFactory::GetPolylineSegments(const TColgp_Array1OfPnt& points, TColGeom_SequenceOfCurve& curves, double tolerance)
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
			throw Standard_Failure("The Polyline has no segments");
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}

	pLoggingService->LogWarning("Could not build polyline");
	return;
}

//Builds a wire a collection of segments and trims it
TopoDS_Wire NWireFactory::BuildDirectrix(TColGeom_SequenceOfCurve& segments, double trimStart, double trimEnd, double tolerance)
{


	TopTools_SequenceOfShape edges;
	try
	{
		BRep_Builder builder;
		double parametricLength = 0;
		TopoDS_Wire wire;
		for (auto it = segments.cbegin(); it != segments.cend(); ++it)
		{
			Handle(Geom_Curve) segment = *it;

			double segLength = Abs(segment->LastParameter() - segment->FirstParameter());
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
						parametricLength += (segment->LastParameter() - segment->FirstParameter());
					}
				}
				//make the segment an edge

				if (edges.Length() == 0) //just add the first one
				{
					BRepBuilderAPI_MakeEdge edgeMaker(segment);
					TopoDS_Edge edgeToAdd = edgeMaker.Edge();
					edges.Append(edgeToAdd);
				}
				else //we need to add this segment ot the start of end of the edges
				{

					TopoDS_Edge lastEdge = TopoDS::Edge(edges.Last());
					TopoDS_Vertex vLast = TopExp::LastVertex(lastEdge); //get the vertex at the end of the wire
					gp_Pnt lastPoint = BRep_Tool::Pnt(vLast); //the last point

					TopoDS_Edge firstEdge = TopoDS::Edge(edges.First());
					TopoDS_Vertex vFirst = TopExp::FirstVertex(firstEdge); //get the vertex at the start of the wire
					gp_Pnt firstPoint = BRep_Tool::Pnt(vFirst); //the first point

					gp_Pnt segStartPoint = segment->Value(segment->FirstParameter()); //start and end of segment to add
					gp_Pnt segEndPoint = segment->Value(segment->LastParameter());

					double lastGap = lastPoint.Distance(segStartPoint);
					double firstGap = firstPoint.Distance(segEndPoint);

					if (lastGap < firstGap) //we can just add it on the end
					{
						if (lastGap > tolerance)
							throw Standard_Failure("Segments are not contiguous");
						AdjustVertexTolerance(vLast, lastPoint, segStartPoint, lastGap);
						TopoDS_Vertex segEndVertex;
						builder.MakeVertex(segEndVertex, segEndPoint, Precision::Confusion());
						BRepBuilderAPI_MakeEdge edgeMaker(segment, vLast, segEndVertex);
						edges.Append(edgeMaker.Edge());
					}
					else //we neeed to add it on the start
					{
						if (firstGap > tolerance)
							throw Standard_Failure("Segments are not contiguous");
						AdjustVertexTolerance(vFirst, firstPoint, segEndPoint, firstGap);
						TopoDS_Vertex segStartVertex;
						builder.MakeVertex(segStartVertex, segStartPoint, Precision::Confusion());
						BRepBuilderAPI_MakeEdge edgeMaker(segment, segStartVertex, vFirst);
						edges.Append(edgeMaker.Edge());
					}
				}
				//add the edges to the wire


			}

		}
		builder.MakeWire(wire);
		for (auto it = edges.cbegin(); it != edges.cend(); ++it)
		{
			builder.Add(wire, *it);
		}
		return wire;
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Could not build directrix");
	return _emptyWire;
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
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Could not build rectangle profile def");
	return _emptyWire;
}

TopoDS_Wire NWireFactory::BuildCircleProfileDef(double radius, const gp_Ax22d& position)
{
	try
	{
		
		gp_Ax2 ax2(gp_Pnt(position.Location().X(), position.Location().Y(),0),gp::DZ(), gp_Dir(position.XDirection().X(), position.XDirection().Y(),0));
		Handle(Geom_Circle) hCirc = GC_MakeCircle(ax2,radius);
		
		TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hCirc);
		return BRepBuilderAPI_MakeWire(edge);		
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Could not build circle profile def");
	return _emptyWire;
}
