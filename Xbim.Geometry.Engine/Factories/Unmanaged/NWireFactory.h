#pragma once
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
#include <TColGeom_SequenceOfCurve.hxx>



class NWireFactory
{
private:
	NLoggingService* pLoggingService;
	TopoDS_Wire _emptyWire;
public:
	NWireFactory()
	{
		pLoggingService = new NLoggingService();
		TopoDS_Builder builder;
		builder.MakeWire(_emptyWire); //make an empty wire for failing operations
	};
	~NWireFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	//Builds a polyline in the context of 0  or more existing vertices, if buildRaw is true no geometrical or topological corrections are made
	TopoDS_Wire BuildPolyline2d(
		const NCollection_Vector<KeyedPnt2d>& pointSeq,		
		double tolerance,bool buildRaw = false);
	TopoDS_Wire BuildPolyline(
		const NCollection_Vector<KeyedPnt>& pointSeq, double startParam, double endParam,
		double tolerance);

	void GetPolylineSegments(const TColgp_Array1OfPnt& points, TColGeom_SequenceOfCurve& curves, double tolerance);

	TopoDS_Wire BuildDirectrix(TColGeom_SequenceOfCurve& segments, double trimStart, double trimEnd, double tolerance);

	void AdjustVertexTolerance(TopoDS_Vertex& vertexToJoinTo, gp_Pnt pointToJoinTo, gp_Pnt pointToJoin, double gap);
	TopoDS_Wire BuildRectangleProfileDef(double xDim, double yDim);
	TopoDS_Wire BuildCircleProfileDef(double radius, const gp_Ax22d& position);
	/// <summary>
	/// calucates the normal of the wire
	/// </summary>
	/// <param name="wire"></param>
	/// <param name="normal"></param>
	/// <returns>true if the normal is valid and has a magnitude, otherwise false</returns>
	bool GetNormal(const TopoDS_Wire& wire, gp_Vec& normal);
	
	double Area(const TopoDS_Wire& wire);
	
};

