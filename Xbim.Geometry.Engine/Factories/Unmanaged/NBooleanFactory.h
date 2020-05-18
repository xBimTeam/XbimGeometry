#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"
#include <TopoDS_Solid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
class NBooleanFactory
{
private:
	double _fuzzyTolerance = 0;
	NLoggingService* pLoggingService;
	TopoDS_Solid _emptySolid;
public:
	NBooleanFactory()
	{		
		TopoDS_Builder builder;
		builder.MakeSolid(_emptySolid); //make an empty solid for failing operations
	};
	~NBooleanFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	void SetFuzzyTolerance(double fuzzyTolerance) { _fuzzyTolerance = fuzzyTolerance; };
	double GetFuzzyTolerance() { return _fuzzyTolerance; };
	//Union of two solids must return a solid
	TopoDS_Shape Union(const TopoDS_Solid& left, const TopoDS_Solid& right);
	//Difference between two solids
	TopoDS_Shape Cut(const TopoDS_Solid& left, const TopoDS_Solid& right);
	//Intersection of two shapes
	TopoDS_Shape Intersect(const TopoDS_Solid& left, const TopoDS_Solid& right);
};

