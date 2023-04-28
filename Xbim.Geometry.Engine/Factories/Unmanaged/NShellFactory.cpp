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
#include <ShapeFix_Edge.hxx>
#include <BRepCheck_Face.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
#include <GeomPlate_BuildAveragePlane.hxx>
#include <TColGeom_SequenceOfSurface.hxx>
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
TopoDS_Shell NShellFactory::BuildConnectedFaceSet(const std::vector<std::vector<std::vector<int>>>& faceData, const std::unordered_map<int, gp_XYZ>& points, const std::vector<int>& planeIndices, const TColgp_SequenceOfAx1& planes, double tolerance, double minGap, bool& needsFixing)
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
		TColGeom_SequenceOfSurface planarSurfaces;
		int faceIndex = 0;
		for (const std::vector<std::vector<int>>& face : faceData)
		{
			bool hasDefinedPlane = planeIndices[faceIndex] > 0;
			faces.push_back(std::vector<std::vector<int>>());
			std::vector<std::vector<int>>& faceBounds = faces.back();
			int numPointsOnFace = 0;
			for (const std::vector<int>& bounds : face)
				numPointsOnFace += (int)bounds.size();
			int pointIndex = 1;
			Handle(TColgp_HArray1OfPnt) pointsOnFace = new TColgp_HArray1OfPnt(1, numPointsOnFace);
			for (const std::vector<int>& bounds : face)
			{
				faceBounds.push_back(std::vector<int>());
				std::vector<int>& boundVertexIndices = faceBounds.back();
				for (size_t i = 0; i < bounds.size(); i++)
				{
					int idx = bounds[i];
					auto found = points.find(idx);
					gp_XYZ coord = found->second;
					if (!hasDefinedPlane)
						pointsOnFace->SetValue(pointIndex++, gp_Pnt(coord));
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
			if (hasDefinedPlane)
			{
				auto ax1 = planes.Value(faceIndex+1);
				auto thePlane = new Geom_Plane(ax1.Location(), ax1.Direction());
				planarSurfaces.Append(thePlane);
			}
			else
			{
				//we have all the points on the face get a plane to fit them
				GeomPlate_BuildAveragePlane averagePlaneBuilder(pointsOnFace, pointsOnFace->Size(), tolerance, 1, 2);
				if (!averagePlaneBuilder.IsPlane())
					planarSurfaces.Append(Handle(Geom_Plane)());
				else
					planarSurfaces.Append(averagePlaneBuilder.Plane());
			}
			faceIndex++;
		}

		//collect unique set of edges
		std::unordered_map<EdgeId, int, EdgeIdHash, EdgeIdEqual> uniqueEdges;
		TopTools_SequenceOfShape edges;

		int faceCount = 0;
		int facePlaneIndex = 1;
		
		for (const std::vector<std::vector<int>>& face : faces)
		{
			TopoDS_Face theFace;
			auto thePlane = Handle(Geom_Plane)::DownCast(planarSurfaces.Value(facePlaneIndex++));
			builder.MakeFace(theFace, thePlane, tolerance);
			//get the bounds for the face
			for (auto&& bound : face)
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
					aVert = bVert;
					a = b;
				}
				//add the edge if it has an area (more than 2 edges)
				if (topoWire.NbChildren() > 2)
					builder.Add(theFace, topoWire);
			}

			if (theFace.NbChildren() == 0)
				continue; //nothing to do
			//find the outer bound
			auto outerBound = BRepTools::OuterWire(theFace);
			//make sure the outer bound is within tolerance of the face
			ShapeFix_Wire wireFixer(outerBound, theFace, tolerance);
			wireFixer.ClearModes();
			wireFixer.FixVertexToleranceMode() = true;
			wireFixer.Perform();
			//it is possible the average plane builder may have built the plane reversed, ensure the puter bound is counter clock wise winding
			//if not reverse the plane to comply with the topology
			for (TopExp_Explorer exp(theFace,TopAbs_WIRE);exp.More();exp.Next())
			{
				auto&& wire = TopoDS::Wire(exp.Current());
				if (wire.IsEqual(outerBound))
				{
					TopoDS_Face tmpFace;
					builder.MakeFace(tmpFace, thePlane, tolerance);
					builder.Add(tmpFace, wire);
					GProp_GProps gProps;
					BRepGProp::SurfaceProperties(tmpFace, gProps, tolerance);
					double area = gProps.Mass();
					if (std::abs(area) < Precision::Confusion())
					{
						theFace.EmptyCopy(); //no valid outer wire, maybe just a line
						break;
					}
					bool isCounterClockwise = area > 0;
					if (!isCounterClockwise)
						thePlane->SetAxis(thePlane->Axis().Reversed());
					break;
				}
			}
			if (theFace.NbChildren() == 0)
				continue; //nothing to do
			if (theFace.NbChildren() > 1)
			{
				bool faceNeedsToBeRebuilt = false;
				TopoDS_ListOfShape innerWires;
				//if there are inner loops ensure they are clockwise oreinted to maintain correct topology
				for (TopExp_Explorer exp(theFace, TopAbs_WIRE); exp.More(); exp.Next())
				{
					const TopoDS_Shape& wire = exp.Current();
					if (!wire.IsEqual(outerBound))
					{
						//fix the tolerance
						wireFixer.Init(TopoDS::Wire(wire), theFace, tolerance);
						wireFixer.Perform();
						TopoDS_Face tmpFace;
						builder.MakeFace(tmpFace, thePlane, tolerance);
						builder.Add(tmpFace, wire);
						GProp_GProps gProps;
						BRepGProp::SurfaceProperties(tmpFace, gProps, tolerance);
						double area = gProps.Mass();
						if (std::abs(area) < Precision::Confusion())
							continue;
						bool isCounterClockwise = area > 0;
						if (isCounterClockwise)
						{
							innerWires.Append(wire.Reversed()); 
							faceNeedsToBeRebuilt = true;
						}
						else
							innerWires.Append(wire);					
					}
				}
				if (faceNeedsToBeRebuilt)
				{
					//Rebuld the face with the modified wires
					theFace.EmptyCopy();
					builder.Add(theFace, outerBound);
					for (auto&& innerWire: innerWires)
						builder.Add(theFace, innerWire);
				}
			}
			//BRepBuilderAPI_FindPlane planeFinder1(outerBound, tol / 10);
			//if (!planeFinder1.Found()) //the wire is planar
			//{
			//	int r = 1;
			//	TopoDS_Face tmpFace;
			//	builder.MakeFace(tmpFace, thePlane, tolerance);
			//	builder.Add(tmpFace, outerBound);
			//	GProp_GProps gProps;
			//	BRepGProp::SurfaceProperties(tmpFace, gProps, tolerance);
			//	double area = gProps.Mass();
			//	BRepTools::Write(theFace, "/tmp/2.brep");
			//	r++;
			//}


			//build the face	
			faceCount++;
			builder.Add(shell, theFace);
		}
		return shell;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);

	}
	return TopoDS_Shell();
}
