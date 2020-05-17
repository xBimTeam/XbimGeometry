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
	return _emptySolid;
}
