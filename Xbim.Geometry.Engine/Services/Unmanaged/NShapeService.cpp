#include "NShapeService.h"
#include <BOPAlgo_BOP.hxx>
#include "NProgressMonitor.h"

 
TopoDS_Shape NShapeService::Cut(const TopoDS_Shape& body, const TopTools_ListOfShape& subtractions, double precision)
{
	try
	{

		BOPAlgo_BOP aBOP;
		aBOP.AddArgument(body);
		aBOP.SetTools(subtractions);
		aBOP.SetOperation(BOPAlgo_CUT);
		aBOP.SetRunParallel(false);
		aBOP.SetNonDestructive(true);
		aBOP.SetFuzzyValue(precision);
		NProgressMonitor pi(_timeout);

		TopoDS_Shape aR;
		aBOP.Perform(pi);
		aR = aBOP.Shape();
		return aR;
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
	}
	return TopoDS_Shape();
}

TopoDS_Shape NShapeService::Cut(const TopoDS_Shape& body, const TopoDS_Shape& subtraction, double precision)
{
	TopTools_ListOfShape subtractions;
	subtractions.Append(subtraction);
	return Cut(body, subtractions, precision);
}

TopoDS_Shape NShapeService::Union(const TopoDS_Shape& body, const TopTools_ListOfShape& additions, double precision)
{
	try
	{
		BOPAlgo_BOP aBOP;
		aBOP.AddArgument(body);
		aBOP.SetTools(additions);
		aBOP.SetOperation(BOPAlgo_FUSE);
		aBOP.SetRunParallel(false);
		aBOP.SetNonDestructive(true);
		aBOP.SetFuzzyValue(precision);
		NProgressMonitor pi(_timeout);

		TopoDS_Shape aR;
		aBOP.Perform(pi);
		aR = aBOP.Shape();
		return aR;
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm); 
	}
	return TopoDS_Shape();
}
 
TopoDS_Shape NShapeService::Union(const TopoDS_Shape& body, const TopoDS_Shape& addition, double precision)
{
	TopTools_ListOfShape additions;
	additions.Append(addition);
	return Union(body, additions, precision);
}

TopoDS_Shape NShapeService::Intersect(const TopoDS_Shape& body, const TopTools_ListOfShape& otherShapes, double precision)
{
	try
	{
		BOPAlgo_BOP aBOP;
		aBOP.AddArgument(body);
		aBOP.SetTools(otherShapes);
		aBOP.SetOperation(BOPAlgo_COMMON);
		aBOP.SetRunParallel(false);
		aBOP.SetNonDestructive(true);
		aBOP.SetFuzzyValue(precision);
		NProgressMonitor pi(_timeout);

		TopoDS_Shape aR;
		aBOP.Perform(pi);
		aR = aBOP.Shape();
		return aR;
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm); 
	}
	return TopoDS_Shape();
}

TopoDS_Shape NShapeService::Intersect(const TopoDS_Shape& body, const TopoDS_Shape& otherShape, double precision)
{
	TopTools_ListOfShape otherShapes;
	otherShapes.Append(otherShape);
	return Union(body, otherShapes, precision);
}