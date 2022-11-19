#pragma once
#include "../../Services//Unmanaged/NLoggingService.h"
#include <TopoDS_Edge.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <BRep_Builder.hxx>
#include "NCurveFactory.h"
class NEdgeFactory
{
private:

	NLoggingService* pLoggingService;
	NCurveFactory _curveFactory;
public:

	NEdgeFactory()
	{
		pLoggingService = nullptr;		
	};
	~NEdgeFactory()
	{
		if(pLoggingService!=nullptr) delete pLoggingService;
		pLoggingService = nullptr;
		
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Edge BuildEdge(Handle(Geom_Curve) hCurve);
	TopoDS_Edge BuildEdge(Handle(Geom2d_Curve) hCurve);
	TopoDS_Edge BuildEdge(const gp_Pnt& start, const gp_Pnt& end);
};

