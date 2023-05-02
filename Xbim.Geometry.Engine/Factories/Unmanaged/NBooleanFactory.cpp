#include "NBooleanFactory.h"

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include "../../XbimProgressMonitor.h"
#include <BOPAlgo_PaveFiller.hxx>
#include <Standard_Type.hxx>
#include <BOPAlgo_Alerts.hxx>
#include <ShapeFix_Shape.hxx>
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
		hasWarnings = true;
		return TopoDS_Shape();
	}
	if (IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Union two solids, the right one is empty. Result is the left solid");
		hasWarnings = true;
		return left;
	}
	if (IsEmpty(left))
	{
		pLoggingService->LogWarning("Attempt to Union two solids, the left one is empty. Result is the right solid");
		hasWarnings = true;
		return right;
	}
	TopoDS_ListOfShape arguments;
	TopoDS_ListOfShape tools;
	arguments.Append(left);
	tools.Append(right);
	return PerformBoolean(arguments, tools, fuzzyTolerance, BOPAlgo_FUSE, hasWarnings);
}


TopoDS_Shape NBooleanFactory::Cut(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings)
{
	//Cutting two solids must return one or more solids unless left and right are both empty shapes
	//or the cut removes all content
	if (IsEmpty(left) && IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Cut two empty solids. Result is an empty solid");
		hasWarnings = true;
		return TopoDS_Shape();
	}
	if (IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the right one is empty. Result is the left solid");
		hasWarnings = true;
		return left;
	}
	if (IsEmpty(left))
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the left one is empty. Result is an empty solid");
		hasWarnings = true;
		return TopoDS_Shape();
	}
	TopoDS_ListOfShape arguments;
	TopoDS_ListOfShape tools;
	arguments.Append(left);
	tools.Append(right);
	return PerformBoolean(arguments, tools, fuzzyTolerance, BOPAlgo_CUT, hasWarnings);
}


TopoDS_Shape NBooleanFactory::Intersect(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance, bool& hasWarnings)
{
	//Intersection of two solids can  return one or no solids if left or right are empty shapes
	//then there can be no intersection
	if (IsEmpty(left) || IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Intersect one or more empty solids. Result is an empty solid");
		hasWarnings = true;
		return TopoDS_Shape();
	}
	TopoDS_ListOfShape arguments;
	TopoDS_ListOfShape tools;
	arguments.Append(left);
	tools.Append(right);
	return PerformBoolean(arguments, tools, fuzzyTolerance, BOPAlgo_COMMON, hasWarnings);
}
TopoDS_Shape NBooleanFactory::PerformBoolean(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double fuzzyTolerance, BOPAlgo_Operation operation, bool& hasWarnings)
{
	return PerformBoolean(arguments, tools, fuzzyTolerance, operation, hasWarnings, false);
}
TopoDS_Shape NBooleanFactory::PerformBoolean(const TopoDS_ListOfShape& arguments, const TopoDS_ListOfShape& tools, double fuzzyTolerance, BOPAlgo_Operation operation, bool& hasWarnings, bool attemptingFix)
{
	//try and operate
	try
	{
		hasWarnings = false;
		BRepAlgoAPI_BooleanOperation bop;
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(operation);
		bop.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		bop.SetNonDestructive(true);
		bop.SetFuzzyValue(fuzzyTolerance);
		//bop.SetGlue(BOPAlgo_GlueShift);

		XbimProgressMonitor pi(Timout);
		bop.Build(pi);
		if (pi.UserBreak())
		{
			Standard_Failure::Raise("Boolean operation timed out");
		}


		//if (bop.HasWarnings())
		//{
		//	hasWarnings = true;
		//	std::ostringstream msg;
		//	//bop.DumpWarnings(msg);
		//	auto report = bop.GetReport();
		//	report->Dump(msg);
		//	pLoggingService->LogDebug(msg.str().c_str());
		//}
		if (bop.HasErrors())
		{
			std::ostringstream msg;
			//bop.DumpErrors(msg);
			auto& report = bop.GetReport();
			report->Dump(msg);
			Standard_Failure::Raise(msg.str().c_str());
		}


		if (bop.IsDone()) //work out what to do in this situation
		{
			
			bop.SimplifyResult(true, true, Precision::Angular());
			//if we have a self intersection acquired it means one of the input shapes had a self intersection, fix up the input shapes and repeat, if we have not tried to do so before
			if (bop.DSFiller()->HasWarning(STANDARD_TYPE(BOPAlgo_AlertAcquiredSelfIntersection)) && !attemptingFix)
			{
				
				TopoDS_ListOfShape fixedArguments;
				TopoDS_ListOfShape fixedTools;
				for (auto&& argument : arguments)
				{
					ShapeFix_Shape shapeFixer(argument);
					if (shapeFixer.Perform())
						fixedArguments.Append(shapeFixer.Shape());
					else
						fixedArguments.Append(argument);
				}
				for (auto&& tool : tools)
				{
					ShapeFix_Shape shapeFixer(tool);
					if (shapeFixer.Perform())
						fixedTools.Append(shapeFixer.Shape());
					else
						fixedTools.Append(tool);
				}
				return PerformBoolean(fixedArguments, fixedTools, fuzzyTolerance, operation, hasWarnings, true);
			}
			if(attemptingFix)
				pLoggingService->LogDebug("Self-intersection of sub-shapes in the Boolean Operations output results has been fixed.");
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
