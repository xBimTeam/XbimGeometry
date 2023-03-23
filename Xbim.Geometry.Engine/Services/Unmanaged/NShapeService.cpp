#include "NShapeService.h"
#include <BOPAlgo_BOP.hxx>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include "NProgressMonitor.h"
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <TopoDS_Iterator.hxx>
#include "../../XbimProgressMonitor.h"

 
TopoDS_Shape NShapeService::Cut(const TopoDS_Shape& body, const TopoDS_ListOfShape& subtractions, double precision)
{
	TopoDS_ListOfShape arguments; 
	arguments.Append(body); 
	bool hasWarnings;
	return PerformBoolean(arguments, subtractions, precision, BOPAlgo_CUT, hasWarnings);
}


TopoDS_Shape NShapeService::Cut(const TopoDS_Shape& body, const TopoDS_Shape& subtraction, double precision)
{
	TopoDS_ListOfShape arguments;
	TopoDS_ListOfShape tools;
	arguments.Append(body);
	tools.Append(subtraction);
	bool hasWarnings;
	return PerformBoolean(arguments, tools, precision, BOPAlgo_CUT, hasWarnings);
}


TopoDS_Shape NShapeService::Union(const TopoDS_Shape& body, const TopoDS_ListOfShape& additions, double precision)
{
	TopoDS_ListOfShape arguments;
	arguments.Append(body);
	bool hasWarnings;
	return PerformBoolean(arguments, additions, precision, BOPAlgo_FUSE, hasWarnings);
}
 

TopoDS_Shape NShapeService::Union(const TopoDS_Shape& body, const TopoDS_Shape& addition, double precision)
{
	TopoDS_ListOfShape arguments;
	TopoDS_ListOfShape tools;
	arguments.Append(body);
	tools.Append(addition);
	bool hasWarnings;
	return PerformBoolean(arguments, tools, precision, BOPAlgo_FUSE, hasWarnings);
}


TopoDS_Shape NShapeService::Intersect(const TopoDS_Shape& body, const TopoDS_ListOfShape& otherShapes, double precision)
{
	TopoDS_ListOfShape arguments;
	arguments.Append(body);
	bool hasWarnings;
	return PerformBoolean(arguments, otherShapes, precision, BOPAlgo_COMMON, hasWarnings);
}


TopoDS_Shape NShapeService::Intersect(const TopoDS_Shape& body, const TopoDS_Shape& otherShape, double precision)
{
	TopoDS_ListOfShape arguments;
	TopoDS_ListOfShape tools;
	arguments.Append(body);
	tools.Append(otherShape);
	bool hasWarnings;
	return PerformBoolean(arguments, tools, precision, BOPAlgo_COMMON, hasWarnings);
}


TopoDS_Shape NShapeService::PerformBoolean(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double fuzzyTolerance, BOPAlgo_Operation operation, bool& hasWarnings)
{
	try
	{
		hasWarnings = false;
		BRepAlgoAPI_BooleanOperation bop;
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(operation);
		bop.SetRunParallel(false);
		bop.SetNonDestructive(true);
		bop.SetFuzzyValue(fuzzyTolerance);

		XbimProgressMonitor pi(_timeout);
		bop.Build(pi);
		if (bop.HasWarnings())
		{
			hasWarnings = true;
			std::ostringstream msg;
			bop.DumpWarnings(msg);
			pLoggingService->LogWarning(msg.str().c_str());
		}
		if (bop.HasErrors())
		{
			std::ostringstream msg;
			bop.DumpErrors(msg);
			Standard_Failure::Raise(msg.str().c_str());
		}
		if (pi.UserBreak())
		{
			Standard_Failure::Raise("Boolean operation timed out");
		}

		if (bop.IsDone()) //work out what to do in this situation
		{
			bop.SimplifyResult(true, true, Precision::Angular());
			return TrimTopology(bop.Shape());
		}

	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Failed to perform boolean");
	return TopoDS_Shape();
}


TopoDS_Shape NShapeService::TrimTopology(const TopoDS_Shape& shape)
{
	if (shape.ShapeType() != TopAbs_COMPOUND || shape.NbChildren() != 1) return shape;//more than one top level shape cannot be simplified
	TopoDS_Iterator exp(shape);
	auto child = exp.Value();
	return TrimTopology(child);
}


TopoDS_Shape NShapeService::Transform(const TopoDS_Shape& shape, const gp_GTrsf& trans)
{
	BRepBuilderAPI_GTransform gTran(shape, trans, Standard_True);
	return gTran.Shape();
}


void NShapeService::LogStandardFailure(const Standard_Failure& e, char* additionalMessage)
{
	std::stringstream strm;
	strm << additionalMessage << std::endl;
	e.Print(strm);
	pLoggingService->LogError(strm.str().c_str());
}


void NShapeService::LogStandardFailure(const Standard_Failure& e)
{
	std::stringstream strm;
	e.Print(strm);
	pLoggingService->LogError(strm.str().c_str());
}