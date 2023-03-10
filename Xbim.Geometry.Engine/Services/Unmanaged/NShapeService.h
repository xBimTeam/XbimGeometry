#pragma once
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>

class NShapeService
{
private:
	double _timeout;
public:
	NShapeService(double timeout) : _timeout(timeout) {};
	TopoDS_Shape Cut(const TopoDS_Shape& body, const TopoDS_Shape& subtraction, double precision);
	TopoDS_Shape Union(const TopoDS_Shape& body, const TopoDS_Shape& addition, double precision);
	TopoDS_Shape Intersect(const TopoDS_Shape& body, const TopoDS_Shape& other, double precision);
	TopoDS_Shape Cut(const TopoDS_Shape& body, const TopTools_ListOfShape& subtractions, double precision);
	TopoDS_Shape Union(const TopoDS_Shape& body, const TopTools_ListOfShape& additions, double precision);
	TopoDS_Shape Intersect(const TopoDS_Shape& body, const TopTools_ListOfShape& others, double precision);
	TopoDS_Shape Transform(const TopoDS_Shape& shape, const gp_GTrsf& trans);
};

