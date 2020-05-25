#include "NWireFactory.h"
#include <TopoDS.hxx>
#include <gp_Lin2d.hxx>
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
		size_t desiredPointCount = closed ? pointSeq.Length() - 1 : pointSeq.Length();
		size_t actualPointCount = vertices.Size();
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
		for (size_t i = 2; i <= actualPointCount; i++)
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

		for (size_t i = 1; i < pointSeq.Length(); i++)
		{
			const KeyedPnt& startKP = pointSeq.Value(i - 1);
			const KeyedPnt& endKP = pointSeq.Value(i);
			int vCount = vertices.Length();

			gp_Pnt startPoint = vCount == 0 ? startKP.myPnt : BRep_Tool::Pnt(TopoDS::Vertex(vertices.Last()));
			double pointTolerance = tolerance + (vCount == 0 ?Precision::Confusion() : BRep_Tool::Tolerance(TopoDS::Vertex(vertices.Last())));
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
			edgeVec.Normalize();
			if (startParam >= 0) //we want to clip, adjust the vertices if necessary
			{
				if (startParam >= offset && startParam < offset + segLength) //trim this edge its the first one, will only enter the first time
				{
					startPoint.Translate(edgeVec * (startParam - offset)); //move the start point	
					TopoDS_Vertex startVertex;
					builder.MakeVertex(startVertex, startPoint, Precision::Confusion());
					vertices.Append(startVertex);
					//check if it is also the last one
					if (endParam <= offset + segLength)
					{
						//trim this edge to start and end and give in
						endPoint = startPoint.Translated(edgeVec * (endParam - offset));
						TopoDS_Vertex endVertex;
						builder.MakeVertex(endVertex, endPoint, Precision::Confusion());
						vertices.Append(endVertex);
						break; //all done
					}
					startParam = -1; //don't look for anymore start edges

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

		for (size_t i = 2; i <= vertices.Length(); i++)
		{
			BRepBuilderAPI_MakeEdge edgeMaker(TopoDS::Vertex(vertices.Value(i - 1)), TopoDS::Vertex(vertices.Value(i)));
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
