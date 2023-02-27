#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Solid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>

class NBooleanFactory : public NFactoryBase
{
	
public:
	double Timout = 20; //seconds

	//Union of two solids must return a solid
	TopoDS_Shape Union(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings);
	//Difference between two solids
	TopoDS_Shape Cut(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings);
	//Intersection of two shapes
	TopoDS_Shape Intersect(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings);
	static bool IsEmpty(const TopoDS_Shape& shape);
	/*TopoDS_Shape Union(const TopTools_ListOfShape& shapes, double fuzzyTolerance);*/
	TopoDS_Shape TrimTopology(const TopoDS_Shape& shape);
	TopoDS_Shape PerformBoolean(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double fuzzyTolerance, BOPAlgo_Operation operation, bool& hasWarnings);
};

