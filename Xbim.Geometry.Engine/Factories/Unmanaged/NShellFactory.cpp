#include "NShellFactory.h"

#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <BRepBuilderAPI_VertexInspector.hxx>
#include <TopoDS.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopExp.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <ShapeAnalysis.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <gp_Pln.hxx>
#include <BOPAlgo_Tools.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools.hxx>
#include <BRepGProp_Face.hxx>
#include <BRepLib_FindSurface.hxx>
#include <Geom_Plane.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepFeat.hxx>
#include <gp_Pln.hxx>
#include <Geom_Line.hxx>
#include <gce_MakeLin.hxx>
#include <ShapeFix_Wire.hxx>
#include <unordered_map>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepCheck_Face.hxx>
#include <BRepCheck_Analyzer.hxx>
struct EdgeId
{
public:
	const int Start;
	const int End;
	int LowIndex() const { return Reversed ? End : Start; };
	int HighIndex()const { return Reversed ? Start : End; };
	const bool Reversed;
	EdgeId(int a, int b) : Start(a), End(b), Reversed(a > b) {}
	const bool IsValid() { return Start != End; }
	int Hash() const { return LowIndex() ^ (HighIndex() << 1); }
	bool Equals(const EdgeId& rhs)const
	{
		//and edge Id is unique regardless of the order of the low and high index
		return	(Start == rhs.Start && End == rhs.End) ||
			(Start == rhs.End && End == rhs.Start);
	}
};

struct EdgeIdHash {
	std::size_t operator()(const EdgeId& k) const
	{
		return k.Hash();
	}
};

struct EdgeIdEqual {
	bool operator()(const EdgeId& lhs, const EdgeId& rhs) const
	{
		//and edge Id is unique regardless of the order of the low and high index
		return lhs.Equals(rhs);
	}
};
/// <summary>
/// the loops are a list of faces, which are described as a list of loops (inner and outer bounds) as a list of XYZ attributes
/// </summary>
/// <param name="loops"></param>
/// <returns></returns>
TopoDS_Shell NShellFactory::BuildConnectedFaceSet(const std::vector<std::vector<std::vector<int>>>& faceData, const std::unordered_map<int, gp_XYZ>& points, double tolerance, double minGap, bool& needsFixing)
{
	static int counter = 0;
	needsFixing = false;
	//coursen the tolerance as often these shapes are outputs of previous boolean operations in the BIM tool. The boolean operations in Revit are often inaccurate to up to 1 mm 
	double tol = minGap > tolerance ? minGap : tolerance;
	BRepBuilderAPI_VertexInspector inspector(tol);
	NCollection_CellFilter<BRepBuilderAPI_VertexInspector> vertexCellFilter(2, tol);

	TopoDS_Shell shell;
	BRep_Builder builder;
	builder.MakeShell(shell);
	try
	{
		//collect unique set of vertices
		TopTools_SequenceOfShape vertices;
		std::vector<std::vector<std::vector<int>>> faces;

		for (const std::vector<std::vector<int>>& face : faceData)
		{
			faces.push_back(std::vector<std::vector<int>>());
			std::vector<std::vector<int>>& faceBounds = faces.back();
			for (const std::vector<int>& bounds : face)
			{
				faceBounds.push_back(std::vector<int>());
				std::vector<int>& boundVertexIndices = faceBounds.back();
				for (size_t i = 0; i < bounds.size(); i++)
				{
					int idx = bounds[i];
					auto found = points.find(idx);
					gp_XYZ coord = found->second;
					inspector.ClearResList();
					inspector.SetCurrent(coord);
					vertexCellFilter.Inspect(coord, inspector);
					const TColStd_ListOfInteger& results = inspector.ResInd();
					TopoDS_Vertex vertex;
					if (results.Size() > 0) //hit
					{
						//just take the first one as we don't add vertices more than once to a cell
						int vertexIdx = results.First();
						vertex = TopoDS::Vertex(vertices.Value(vertexIdx));
						boundVertexIndices.push_back(vertexIdx);
					}
					else //miss
					{
						inspector.Add(coord);
						//build the vertex
						//see for tolerance explaination https://opencascade.blogspot.com/2009/02/topology-and-geometry-in-open-cascade_09.html
						//nb if building from ground up use maximum precision
						builder.MakeVertex(vertex, coord, Precision::Confusion()); //use highest precision as we have already made all required vertices coincidental using the inspector
						vertices.Append(vertex); //it will have the same index as the point in the inspector
						vertexCellFilter.Add(vertices.Size(), coord);
						boundVertexIndices.push_back(vertices.Size());
					}

				}
			}
		}

		//collect unique set of edges
		std::unordered_map<EdgeId, int, EdgeIdHash, EdgeIdEqual> uniqueEdges;
		TopTools_SequenceOfShape edges;


		int faceCount = 0;
		for (const std::vector<std::vector<int>>& face : faces)
		{
			TopoDS_Wire outerBound;
			std::vector<TopoDS_Wire> topoWires;
			std::vector<gp_Dir> topoWireNornals;
			double largestArea = 0;
			//get the bounds for the face
			for (const std::vector<int>& bound : face)
			{
				if (bound.empty())
					continue;

				TopoDS_Wire topoWire;
				builder.MakeWire(topoWire);
				//get each vertex pair
				auto& vertexIt = bound.cbegin();
				int a = *vertexIt;
				TopoDS_Vertex aVert = TopoDS::Vertex(vertices.Value(a));
				vertexIt++;
				for (; vertexIt != bound.cend(); vertexIt++)
				{
					int b = *vertexIt;
					EdgeId edgeId(a, b);
					TopoDS_Vertex bVert = TopoDS::Vertex(vertices.Value(b));
					if (edgeId.IsValid())
					{
						//just add unique edges, no need for the reversed versions
						auto& inserted = uniqueEdges.try_emplace(edgeId, edges.Size() + 1);

						if (inserted.second) //it was create in a->b orientation
						{
							BRepBuilderAPI_MakeEdge edgeMaker(aVert, bVert);
							TopoDS_Edge edge = edgeMaker.Edge();
							edges.Append(edge);
							builder.Add(topoWire, edge);
						}
						else //we already have the forward oriented version
						{
							TopoDS_Edge edge = TopoDS::Edge(edges.Value(inserted.first->second));
							const EdgeId foundEdgeId = inserted.first->first;
							if (foundEdgeId.Start == edgeId.Start)  //the TopoEdge retrieved is same order 
								builder.Add(topoWire, edge);
							else
								builder.Add(topoWire, edge.Reversed());
						}
					}
					//else //we can safely ignore this as it it is not an edge it is a point
					//{
					//	pLoggingService->LogDebug("Zero length edge ignored"); //we can safely just ignore this data as it is an edge between two identical points
					//}
					aVert = bVert;
					a = b;
				}
				double area = std::abs(WireFactory.Area(topoWire));
				topoWires.push_back(topoWire);
				if (area > largestArea)
				{
					largestArea = area;
					outerBound = topoWire;
				}

			}

			//build the face
			gp_Vec wireNormal;
			if (WireFactory.GetNormal(outerBound, wireNormal)) //no normal then unlikely to be a face, most likely co-linear edges
			{
				BRepBuilderAPI_FindPlane planeFinder(outerBound, tol);
				Handle(Geom_Plane) plane;
				TopoDS_Face theFace;
				if (planeFinder.Found()) //the wire is planar
				{
					plane = planeFinder.Plane();
					builder.MakeFace(theFace, plane, tol);
				}
				else //the wire is not planar and will most likely result in more than one face, this happens in some faceted models
				{
					gp_Pnt baryCentre;
					BRepFeat::Barycenter(outerBound, baryCentre);
					plane = new Geom_Plane(baryCentre, wireNormal);
					builder.MakeFace(theFace, plane, tol);
					needsFixing = true;
				}
				
				builder.Add(theFace, outerBound);
				gp_Vec faceNormal = FaceFactory.Normal(theFace);
				bool isOpposite = faceNormal.IsOpposite(wireNormal, 0.1);//srl. some models are quite out of tolerance, we don't need a very precise definition of opposite ~5 degrees is fine
				if (isOpposite)
					theFace.Reverse(); //this should never happen

				//add any inner loops
				if (topoWires.size() > 1) //if we have any inner bounds
				{

					for (auto& wireIt = topoWires.cbegin(); wireIt != topoWires.cend(); wireIt++)
					{
						if (wireIt->IsNotEqual(outerBound))
						{
							TopoDS_Wire hole = *wireIt;
							gp_Vec holeNormal;
							if (WireFactory.GetNormal(hole, holeNormal)) //ignore if invalid hole, they have no normal
							{
								bool isOppositeHole = wireNormal.IsOpposite(holeNormal, 0.1); //srl. we are going to fit this to the plane anyway so we don't need a very precise definition of opposite ~5 degrees is fine								
								if (!isOppositeHole) hole.Reverse();
								//adjust the tolerances of the vertices of the edges to align with the face, Revit often has vertices out of tolerance of the face, fixes pcurves etc							
								ShapeFix_Wire wireFixer(hole, theFace, tolerance);
								bool fixed = wireFixer.FixEdgeCurves();
								builder.Add(theFace, hole);
							}
						}
					}
				}
				//add it to the shell

				faceCount++;
				builder.Add(shell, theFace);
			}
			//else //don't both warning about this it is always a line and/or a face with 0 area
			//{
			//	pLoggingService->LogDebug("Null area face ignored");
			//}
		}
		return shell;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);

	}
	return TopoDS_Shell();
}
