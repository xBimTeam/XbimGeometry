#include "WireFactory.h"
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <cmath>

// Builds a wire from a list of contiguous edges, duplicate vertices at nodes are removed, it is assumed the edges are in contiguous order
int WireFactory::Make(TopTools_ListOfShape& edgeList, double tolerance, double millimeter, TopoDS_Wire& resultWire)
{
	TopoDS_Wire resultWire;
	Builder.MakeWire(resultWire);
	double maxGap = 5 * millimeter;
	bool init = true;
	//iterate over each edge and add to the wire
	for (TopTools_ListIteratorOfListOfShape edgeIterator(edgeList); edgeIterator.More(); edgeIterator.Next())
	{
		const TopoDS_Edge& edge = TopoDS::Edge(edgeIterator.Value()); //will throw na exception if the wrong types are sent
		if (init) //initialise the first edge
		{
			Builder.Add(resultWire, edge);
			init = false;
		}
		else
		{
			TopoDS_Vertex resultWireFirstVertex;
			TopoDS_Vertex resultWireLastVertex;
			gp_Pnt resultWireFirstPoint;
			gp_Pnt resultWireLastPoint;
			TopExp::Vertices(resultWire, resultWireFirstVertex, resultWireLastVertex);
			if (resultWireFirstVertex.IsNull() || resultWireLastVertex.IsNull()) //this should never happen
			{

				//XbimGeometryCreator::LogWarning(logger, cCurve, "Failed to build composite curve. It has been ignored");
				return -1;
			}
			if (resultWireFirstVertex.IsEqual(resultWireLastVertex)) //the wire is closed so give up
			{
				//XbimGeometryCreator::LogWarning(logger, cCurve, "Composite curve is closed. Further segments cannot be added and are ignored");

				return -2;
			}
			resultWireFirstPoint = BRep_Tool::Pnt(resultWireFirstVertex);
			resultWireLastPoint = BRep_Tool::Pnt(resultWireLastVertex);

			TopoDS_Vertex edgeFirstVertex = TopExp::FirstVertex(edge, Standard_True);
			TopoDS_Vertex edgeLastVertex = TopExp::LastVertex(edge, Standard_True);
			gp_Pnt edgeFirstPoint = BRep_Tool::Pnt(edgeFirstVertex);
			gp_Pnt edgeLastPoint = BRep_Tool::Pnt(edgeLastVertex);
			//simple clockwise end of last wire to start of first
			double distFirstToLast = edgeFirstPoint.Distance(resultWireLastPoint);
			double distLastToLast = edgeLastPoint.Distance(resultWireLastPoint);
			double nearest = std::fmin(distFirstToLast, distLastToLast);
			if (nearest == distFirstToLast && nearest < maxGap) //take this first if it is in tolerance
				AddEdge(resultWire, edge, edgeFirstVertex, edgeFirstPoint, edgeLastVertex, resultWireLastVertex, resultWireLastPoint, distFirstToLast);
			else if (distLastToLast < maxGap) //else if this is in tolerance take this
				AddEdge(resultWire, edge, edgeLastVertex, edgeLastPoint, edgeFirstVertex, resultWireLastVertex, resultWireLastPoint, distLastToLast);
			else //see if we have to reverse the edge to get it to attach
			{
				double distLastToFirst = edgeLastPoint.Distance(resultWireFirstPoint);
				double distFirstToFirst = edgeFirstPoint.Distance(resultWireFirstPoint);
				nearest = std::fmin(distLastToFirst, distFirstToFirst);
				if (nearest == distLastToFirst && nearest < maxGap)
					AddEdge(resultWire, TopoDS::Edge(edge.Reversed()), edgeLastVertex, edgeLastPoint, edgeFirstVertex, resultWireFirstVertex, resultWireFirstPoint, distLastToFirst);
				else if (distFirstToFirst < maxGap) //take this if it near enough
					AddEdge(resultWire, TopoDS::Edge(edge.Reversed()), edgeFirstVertex, edgeFirstPoint, edgeLastVertex, resultWireFirstVertex, resultWireFirstPoint, distFirstToFirst);
				else //out of range for attaching
				{
					return -3;
				}
			}
		}
	}
	return 0;
}

//This is going to be added to to the selected vertex and the tolerances will be adjusted. the duplicate points will be removed
void WireFactory::AddEdge(TopoDS_Wire& resultWire, const TopoDS_Edge& edgeToAdd, const TopoDS_Vertex& edgeVertexToJoin, gp_Pnt edgePointToJoin, const TopoDS_Vertex&
	nextEdgeVertex, const TopoDS_Vertex& wireVertexToJoin, gp_Pnt wirePointToJoin, double distance)
{
	TopoDS_Shape emptyEdge = edgeToAdd.EmptyCopied();
	TopoDS_Edge myEdge = TopoDS::Edge(emptyEdge);
	BRep_Builder B;

	Standard_Real tolE, tolW;
	tolW = BRep_Tool::Tolerance(wireVertexToJoin);
	tolE = BRep_Tool::Tolerance(edgeVertexToJoin);
	Standard_Real maxtol = .5 * (tolW + tolE + distance), cW = 1, cE = 0;
	bool adjust = false;
	if (maxtol > tolW && maxtol > tolE)
	{
		cW = (maxtol - tolE) / distance;
		cE = 1. - cW;
		adjust = true;
	}
	else if (maxtol > tolW)
	{
		maxtol = tolE;
		cW = 0.;
		cE = 1.;
		adjust = true;
	}
	if (adjust)
	{
		gp_Pnt PC(cW * wirePointToJoin.X() + cE * edgePointToJoin.X(), cW * wirePointToJoin.Y() + cE * edgePointToJoin.Y(), cW * wirePointToJoin.Z() + cE * edgePointToJoin.Z());
		B.UpdateVertex(wireVertexToJoin, PC, maxtol);
	}
	TopoDS_Vertex firstEdgeVertex = wireVertexToJoin;
	firstEdgeVertex.Orientation(TopAbs_FORWARD);
	B.Add(myEdge, firstEdgeVertex);
	TopoDS_Vertex nextEdgeVertexCopy = nextEdgeVertex;
	nextEdgeVertexCopy.Orientation(TopAbs_REVERSED);
	B.Add(myEdge, nextEdgeVertexCopy);
	B.Transfert(edgeToAdd, myEdge, edgeVertexToJoin, firstEdgeVertex);
	B.Add(resultWire, myEdge);
}

