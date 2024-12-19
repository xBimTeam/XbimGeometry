#pragma once
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include "../Unmanaged/NLoggingService.h"
#include "NWexBimMesh.h"

class NShapeService
{
private:
	NLoggingService* pLoggingService;
	double _timeout;
public:
	NShapeService(double timeout) : _timeout(timeout) {};
	~NShapeService()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	
	TopoDS_Shape Cut(const TopoDS_Shape& body, const TopoDS_Shape& subtraction, double precision);
	TopoDS_Shape Union(const TopoDS_Shape& body, const TopoDS_Shape& addition, double precision);
	TopoDS_Shape Intersect(const TopoDS_Shape& body, const TopoDS_Shape& other, double precision);
	TopoDS_Shape Cut(const TopoDS_Shape& body, const TopoDS_ListOfShape& subtractions, double precision);
	TopoDS_Shape Union(const TopoDS_Shape& body, const TopoDS_ListOfShape& additions, double precision);
	TopoDS_Shape Intersect(const TopoDS_Shape& body, const TopoDS_ListOfShape& others, double precision);
	TopoDS_Shape Transform(const TopoDS_Shape& shape, const gp_GTrsf& trans);

	TopoDS_Shape NShapeService::TrimTopology(const TopoDS_Shape& shape);
	TopoDS_Shape PerformBoolean(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double fuzzyTolerance, BOPAlgo_Operation operation, bool& hasWarnings);
	 
	void SetLogger(WriteLog lFunc)
	{
		NLoggingService* logService = new NLoggingService();
		logService->SetLogger(lFunc);
		pLoggingService = logService;
	};
	void LogStandardFailure(const Standard_Failure& e, char* additionalMessage);
	void LogStandardFailure(const Standard_Failure& e);
	NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale);
};

