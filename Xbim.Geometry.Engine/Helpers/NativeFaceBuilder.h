#pragma once
#include "CanLogBase.h"
#include "gp_Pnt.hxx"
#include "TColgp_SequenceOfPnt.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "GProp_PGProps.hxx"
#include "TopoDS_Wire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "ShapeFix_ShapeTolerance.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "ShapeFix_Edge.hxx"
#include "TopExp.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "ShapeFix_Wire.hxx"
#include "BRepTools.hxx"
#include "BRepGProp_Face.hxx"

#include "XbimNativeApi.h"

class NativeFaceBuilder : public CanLogBase
{
public:
	NativeFaceBuilder(double tolerance, int pointsCount, bool useVertexMap, std::vector<int> handles) : tolerance(tolerance), facePointsCount(pointsCount), useVertexMap(useVertexMap), handles(handles)
	{
	}

	NativeFaceBuilder(double tolerance, int pointsCount) : tolerance(tolerance), facePointsCount(pointsCount), useVertexMap(false)
	{
	}
	NativeFaceBuilder(double tolerance) : tolerance(tolerance), useVertexMap(false)
	{
	}

	TopoDS_Face Init(std::vector<gp_Pnt> points, TopoDS_Wire& theWire, bool& sucess);
	void FixFace(TopoDS_Face& theFace, const std::vector<TopoDS_Face>& innerBoundsList, double angularTolerance, ShapeFix_ShapeTolerance& tolFixer, double tolerance);

private:
	double tolerance;
	int facePointsCount;
	bool useVertexMap;
	std::vector<int> handles;

};