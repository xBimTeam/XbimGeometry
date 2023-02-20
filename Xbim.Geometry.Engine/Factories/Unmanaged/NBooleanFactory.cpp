#include "NBooleanFactory.h"

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include "../../XbimProgressMonitor.h"

bool NBooleanFactory::IsEmpty(const TopoDS_Shape& shape)
{
	return shape.IsNull() || shape.NbChildren() == 0;
}


TopoDS_Shape NBooleanFactory::Union(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings)
{
	//Unioning two solids must return one or more solids unless left and right are both empty shapes
	if (IsEmpty(left) && IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Union two empty solids. Result is an empty solid");
		return TopoDS_Shape();
	}
	if (IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Union two solids, the right one is empty. Result is the left solid");
		return left;
	}
	if (IsEmpty(left))
	{
		pLoggingService->LogWarning("Attempt to Union two solids, the left one is empty. Result is the right solid");
		return right;
	}
	return PerformBoolean(left, right, fuzzyTolerance, BOPAlgo_FUSE, hasWarnings);
}


TopoDS_Shape NBooleanFactory::Cut(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings)
{
	//Cutting two solids must return one or more solids unless left and right are both empty shapes
	//or the cut removes all content
	if (IsEmpty(left) && IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Cut two empty solids. Result is an empty solid");
		return TopoDS_Shape();
	}
	if (IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the right one is empty. Result is the left solid");
		return left;
	}
	if (IsEmpty(left))
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the left one is empty. Result is an empty solid");
		return TopoDS_Shape();
	}
	return PerformBoolean(left, right, fuzzyTolerance, BOPAlgo_CUT, hasWarnings);
}


TopoDS_Shape NBooleanFactory::Intersect(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings)
{
	//Intersection of two solids can  return one or no solids if left or right are empty shapes
	//then there can be no intersection
	if (IsEmpty(left) || IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Intersect one or more empty solids. Result is an empty solid");
		return TopoDS_Shape();
	}
	return PerformBoolean(left, right, fuzzyTolerance, BOPAlgo_COMMON, hasWarnings);
}

TopoDS_Shape NBooleanFactory::PerformBoolean(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, BOPAlgo_Operation operation, bool& hasWarnings)
{
	//try and operate
	try
	{
		hasWarnings = false;
		BRepAlgoAPI_BooleanOperation bop;
		TopTools_ListOfShape arguments;
		TopTools_ListOfShape tools;
		arguments.Append(left);
		tools.Append(right);
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(operation);
		bop.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		bop.SetNonDestructive(true);
//		bop.SetFuzzyValue(fuzzyTolerance);
		//bop.SetGlue(BOPAlgo_GlueFull);
		bop.SimplifyResult();
		XbimProgressMonitor pi(Timout);
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
			return TrimTopology(bop.Shape());
		}

	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Failed to perform boolean");
	return TopoDS_Shape(); //return empty so we fire a managed exception
}
/// <summary>
/// Reduces the shape to its highest level topology, i.e. if a compound contains one solid the solid is returned, only trims compounds
/// </summary>
/// <param name="shape"></param>
/// <returns></returns>
TopoDS_Shape NBooleanFactory::TrimTopology(const TopoDS_Shape& shape)
{
	if (shape.ShapeType() != TopAbs_COMPOUND || shape.NbChildren() != 1) return shape;//more than one top level shape cannot be simplified
	TopoDS_Iterator exp(shape);
	auto child = exp.Value();
	return TrimTopology(child);
}
