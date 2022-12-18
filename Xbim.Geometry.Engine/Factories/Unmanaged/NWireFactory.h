#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Wire.hxx>
#include <TopoDS_Builder.hxx>
#include "../../Services/Unmanaged/NLoggingService.h"
#include <NCollection_Vector.hxx>

#include <gp_XYZ.hxx>
#include <gp_Ax22d.hxx>
#include "../../BRep/OccExtensions/KeyedPnt2d.h"
#include <TopTools_DataMapOfIntegerShape.hxx>
#include "../../BRep/OccExtensions/KeyedPnt.h"
#include <Geom_BSplineCurve.hxx>
#include <TColGeom_SequenceOfBoundedCurve.hxx>
#include <TColGeom2d_SequenceOfBoundedCurve.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
#include <TopTools_SequenceOfShape.hxx>

class NWireFactory : public NFactoryBase
{

public:
	
	bool IsClosed(const TopoDS_Wire& wire, double tolerance);

	TopoDS_Wire BuildWire(const TopoDS_Edge& edge);
	TopoDS_Wire BuildWire(const TopTools_SequenceOfShape& edgeList);
	//Builds a polyline in the context of 0  or more existing vertices, if buildRaw is true no geometrical or topological corrections are made
	TopoDS_Wire BuildPolyline2d(const TColgp_Array1OfPnt2d& points, double tolerance);
	TopoDS_Wire BuildPolyline3d(const TColgp_Array1OfPnt& points, /*double startParam, double endParam,*/ double tolerance);

	


	void GetPolylineSegments3d(const TColgp_Array1OfPnt& points, double tolerance, TColGeom_SequenceOfBoundedCurve& curves);

	void GetPolylineSegments2d(const TColgp_Array1OfPnt2d& points, double tolerance, TColGeom2d_SequenceOfBoundedCurve& curves);

	TopoDS_Wire BuildDirectrixWire(const TopoDS_Wire& wire, double trimStart, double trimEnd, double tolerance, double gapSize);

	TopoDS_Wire BuildTrimmedWire(const TopoDS_Wire& basisWire, double first, double last, bool sameSense, double tolerance);
	
	TopoDS_Wire BuildTrimmedWire(const TopoDS_Wire& basisWire, gp_Pnt p1, gp_Pnt p2, double u1, double u2, bool preferCartesian, bool sameSense, double tolerance);

	TopoDS_Wire BuildWire(const TColGeom2d_SequenceOfBoundedCurve& segments, double tolerance, double gapSize);

	TopoDS_Wire BuildWire(const TColGeom_SequenceOfBoundedCurve& segments, double tolerance, double gapSize);

	bool AdjustVertexTolerance(TopoDS_Vertex& vertexToJoinTo, gp_Pnt pointToJoinTo, gp_Pnt pointToJoin, double gap);
	TopoDS_Wire BuildRectangleProfileDef(double xDim, double yDim);
	TopoDS_Wire BuildCircleProfileDef(double radius, const gp_Ax22d& position);
	TopoDS_Wire BuildOffset(TopoDS_Wire basisWire, double distance);

	TopoDS_Wire BuildOffset(TopoDS_Wire basisWire, double distance, gp_Dir dir);

	//TopoDS_Wire BuildDirectrix(TColGeom_SequenceOfCurve& segments, double trimStart, double trimEnd, double tolerance);


	bool GetParameter(const TopoDS_Wire& wire, gp_Pnt pnt, double tolerance, double& val);
	bool GetNormal(const TopoDS_Wire& wire, gp_Vec& normal);
	
	double Area(const TopoDS_Wire& wire);
	
};

