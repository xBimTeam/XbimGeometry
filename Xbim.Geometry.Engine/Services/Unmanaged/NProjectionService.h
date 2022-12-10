#pragma once
#include "NLoggingService.h"

#include <TopoDS_Edge.hxx>
#include <Poly_Polygon2D.hxx>
#include <TopoDS_Compound.hxx>
#include <TColgp_SequenceOfXY.hxx>
#include <TopTools_ListOfShape.hxx>
#include <NCollection_CellFilter.hxx>
#include <Bnd_Box.hxx>
#include <TColGeom2d_SequenceOfCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include "../../BRep/Unmanaged/NFootprint.h"
#include <Geom_Plane.hxx>
#include <map>
#include <vector>
#include <set>
class BRepMesh_VertexInspector;
typedef NCollection_CellFilter<BRepMesh_VertexInspector> VertexCellFilter;

class NProjectionService
{
private:
	NLoggingService* pLoggingService;

public:
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	void CreateFootPrint(const TopoDS_Shape& shape, double linearDeflection, double angularDeflection, double tolerance, NFootprint& footprint);
	TopoDS_Compound GetOutline(const TopoDS_Shape& shape);
	bool CreateSection(const TopoDS_Shape& shape, const Handle(Geom_Surface)& cutSurface, double tolerance, TopTools_ListOfShape& result);

private:
	void FindOuterLoops(BRepMesh_VertexInspector& anInspector, std::map<int, std::set<int>>& arcs, double linearDeflection, double angularDeflection, double tolerance, NFootprint& footprint);
	TopoDS_Compound FindClosedWires(const TopTools_ListOfShape& edgeList, double tolerance);
	int GetOrAddVertex(
		BRepMesh_VertexInspector& anInspector,
		VertexCellFilter& theCells,
		double x,
		double y,
		double tolerance,
		bool& isNew);
	
	bool UpdateLeftMostPoint(gp_XY& leftMost, double x, double y, double tolerance);
	int FindNextNearestOuterPoint(int currentPointIndex, int previousPointIndex, gp_XY currentPoint, gp_Dir2d currentDir, BRepMesh_VertexInspector& anInspector, const std::map<int, std::set<int>>& arcs);
	void ConnectedPoints(int connectedTo, const std::map<int, std::set<int>>& arcs, std::set<int>& connected);
	bool BuildSegment(gp_XYZ pointA, gp_XYZ pointB, double tolerance, Handle(Geom2d_TrimmedCurve)& segment);
	void ConvertToLinearSegments(const TopoDS_Edge& edge, TColGeom2d_SequenceOfCurve& segments, double tolerance);
};

