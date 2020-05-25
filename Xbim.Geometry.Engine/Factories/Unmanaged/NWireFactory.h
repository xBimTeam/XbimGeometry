#pragma once
#include <TopoDS_Wire.hxx>
#include <TopoDS_Builder.hxx>
#include "../../Services/Unmanaged/NLoggingService.h"
#include <NCollection_Vector.hxx>

#include <gp_XYZ.hxx>
#include "../../BRep/OccExtensions/KeyedPnt2d.h"
#include <TopTools_DataMapOfIntegerShape.hxx>
#include "../../BRep/OccExtensions/KeyedPnt.h"

class NWireFactory
{
private:
	NLoggingService* pLoggingService;
	TopoDS_Wire _emptyWire;
public:
	NWireFactory()
	{
		pLoggingService = nullptr;
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
	
};

