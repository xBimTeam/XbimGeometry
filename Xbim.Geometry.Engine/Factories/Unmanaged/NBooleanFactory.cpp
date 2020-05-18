#include "NBooleanFactory.h"
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <TopoDS.hxx>
TopoDS_Shape NBooleanFactory::Union(const TopoDS_Solid& left, const TopoDS_Solid& right)
{
	//Unioning two solids must return one or more solids unless left and right are both empty shapes
	if (left.IsNull() && right.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Union two empty solids. Result is an empty solid");
		return _emptySolid;
	}
	if (right.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Union two solids, the right one is empty. Result is the left solid");
		return left;
	}
	if (left.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Union two solids, the left one is empty. Result is the right solid");
		return right;
	}
	//try and union
	try
	{
		BRepAlgoAPI_BooleanOperation bop;
		TopTools_ListOfShape arguments;
		TopTools_ListOfShape tools;
		arguments.Append(left);
		tools.Append(right);
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(BOPAlgo_Operation::BOPAlgo_FUSE);
		bop.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		bop.SetNonDestructive(true);
		bop.SetFuzzyValue(_fuzzyTolerance);

		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();	
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/
		
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());		
	}
	pLoggingService->LogWarning("Failed to union solids");
	return _emptySolid; //return empty so we fire a managed exception
}

TopoDS_Shape NBooleanFactory::Cut(const TopoDS_Solid& left, const TopoDS_Solid& right)
{
	//Cutting two solids must return one or more solids unless left and right are both empty shapes
	//or the cut removes all content
	if (left.IsNull() && right.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Cut two empty solids. Result is an empty solid");
		return _emptySolid;
	}
	if (right.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the right one is empty. Result is the left solid");
		return left;
	}
	if (left.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the left one is empty. Result is an empty solid");
		return _emptySolid;
	}
	//try and cut
	try
	{
		BRepAlgoAPI_BooleanOperation bop;
		TopTools_ListOfShape arguments;
		TopTools_ListOfShape tools;
		arguments.Append(left);
		tools.Append(right);
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(BOPAlgo_Operation::BOPAlgo_CUT);
		bop.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		bop.SetNonDestructive(true);
		bop.SetFuzzyValue(_fuzzyTolerance);

		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/

	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Failed to cut solids");
	return _emptySolid; //return empty so we fire a managed exception
}

TopoDS_Shape NBooleanFactory::Intersect(const TopoDS_Solid& left, const TopoDS_Solid& right)
{
	//Intersection of two solids can  return one or no solids if left or right are empty shapes
	//then there can be no intersection
	if (left.IsNull() || right.IsNull())
	{
		pLoggingService->LogWarning("Attempt to Intersect one or more empty solids. Result is an empty solid");
		return _emptySolid;
	}
	//try and intersect
	try
	{
		BRepAlgoAPI_BooleanOperation bop;
		TopTools_ListOfShape arguments;
		TopTools_ListOfShape tools;
		arguments.Append(left);
		tools.Append(right);
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(BOPAlgo_Operation::BOPAlgo_COMMON);
		bop.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		bop.SetNonDestructive(true);
		bop.SetFuzzyValue(_fuzzyTolerance);

		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/

	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Failed to intersect solids");
	return _emptySolid; //return empty so we fire a managed exception
}
