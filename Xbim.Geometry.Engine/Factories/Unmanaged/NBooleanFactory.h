#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Solid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <TopTools_ListOfShape.hxx>
enum Operation
	{
		PerformBoolean, //need to do the operation
		ReturnEmpty, //the result is nothing
		Undefined //the result is nonsense
	};
class NBooleanFactory : public NFactoryBase
{

public:
	
	//Union of two solids must return a solid
	TopoDS_Shape Union(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance);
	//Difference between two solids
	TopoDS_Shape Cut(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance);
	//Intersection of two shapes
	TopoDS_Shape Intersect(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance);
	static bool IsEmpty(const TopoDS_Shape& shape);
	TopoDS_Shape Union(const TopTools_ListOfShape& shapes, double fuzzyTolerance);

	//Operation NBooleanFactory::NextAction(const BOPAlgo_Operation& op, const TopoDS_Shape& left, const TopoDS_Shape& right)
};

