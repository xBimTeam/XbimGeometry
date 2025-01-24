#pragma once

#include <vector>
#include "gp_Pnt.hxx"
#include "BRepTools.hxx"
#include "TopTools_ListOfShape.hxx"
#include "BRepBuilderAPI_VertexInspector.hxx"
#include "TopTools_SequenceOfShape.hxx"
#include "TopTools_DataMapOfShapeListOfShape.hxx"
#include "TopoDS_Vertex.hxx"
#include "BRep_Builder.hxx"
#include "TopoDS.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "TopExp.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "ShapeAnalysis.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRepCheck_Shell.hxx"
#include "BRepCheck_Status.hxx"
#include "../Services/Unmanaged/NLoggingService.h"
#include "CanLogBase.h"


struct FaceBoundParams {
	bool orientation;
	std::vector<gp_Pnt> points;
	bool isOuter;
};

class NativeFacesBuilder : public CanLogBase
{
public:
	NativeFacesBuilder(double precision) : inspector(BRepBuilderAPI_VertexInspector(precision)), tolerance(precision)
	{
	}

	TopoDS_Face ProcessBounds(const std::vector<FaceBoundParams>& params, bool& success);
	TopoDS_Shape BuildShell(const std::vector<TopoDS_Face>& faces);

private:
	double tolerance;
	BRepBuilderAPI_VertexInspector inspector;
	NCollection_CellFilter<BRepBuilderAPI_VertexInspector> vertexCellFilter;
	TopTools_SequenceOfShape vertices;
	TopTools_DataMapOfShapeListOfShape edgeMap;
	BRep_Builder builder; 

};

