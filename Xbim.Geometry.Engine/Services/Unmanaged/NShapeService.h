#pragma once
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include "../Unmanaged/NLoggingService.h"
#include "NWexBimMesh.h"
#include "NShapeProximityUtils.h"

class NShapeService
{
private:
	NLoggingService* pLoggingService;
	NShapeProximityUtils* _proximityUtils;

	double _timeout;
public:
	NShapeService(double timeout) : _timeout(timeout) {
		_proximityUtils = new NShapeProximityUtils();
	};
	~NShapeService()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		if (_proximityUtils != nullptr) delete _proximityUtils;
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
	 
	int GetOverlappingSubShapes1Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection) {
		return _proximityUtils->GetOverlappingSubShapes1Count(shape1, shape2, precision, linearDeflection, angularDeflection);
	}
	int GetOverlappingSubShapes2Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection) {
		return _proximityUtils->GetOverlappingSubShapes2Count(shape1, shape2, precision, linearDeflection, angularDeflection);
	}
	bool IsOverlapping(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double tolerance, double linearDeflection, double angularDeflection) {
		return _proximityUtils->IsOverlapping(shape1, shape2, tolerance, linearDeflection, angularDeflection);
	}

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

