#include "NBooleanFactory.h"
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <TopoDS.hxx>

bool NBooleanFactory::IsEmpty(const TopoDS_Shape& shape)
{
	return shape.IsNull() || shape.NbChildren() == 0;
}
//
//Operation NBooleanFactory::NextAction(const BOPAlgo_Operation& op,  const TopoDS_Shape& left, const TopoDS_Shape& right)
//{
//	TopAbs_ShapeEnum leftType = left.ShapeType();
//	TopAbs_ShapeEnum rightType = right.ShapeType();
//	switch (leftType)
//	{
//	case TopAbs_COMPOUND:
//		break;
//	case TopAbs_COMPSOLID:
//		break;
//	case TopAbs_SOLID:
//		break;
//	case TopAbs_SHELL:
//		break;
//	case TopAbs_FACE:
//		break;
//	case TopAbs_WIRE:
//		break;
//	case TopAbs_EDGE:
//		break;
//	case TopAbs_VERTEX:
//		switch (rightType)
//		{
//		case TopAbs_COMPOUND:
//			break;
//		case TopAbs_COMPSOLID:
//			break;
//		case TopAbs_SOLID:
//			break;
//		case TopAbs_SHELL:
//			break;
//		case TopAbs_FACE:
//			if (op == BOPAlgo_COMMON) return Operation::PerformBoolean;
//			if (op == BOPAlgo_CUT) return Operation::ReturnEmpty;
//			break; //Fuse and Cut21 return undefined
//		case TopAbs_EDGE:
//			if (op == BOPAlgo_COMMON ) return Operation::PerformBoolean; 
//			if(op== BOPAlgo_CUT) return Operation::ReturnEmpty;
//			break; //Fuse and Cut21 return undefined
//		case TopAbs_VERTEX:
//			if (op == BOPAlgo_COMMON || op == BOPAlgo_FUSE) return Operation::PerformBoolean; else return Operation::ReturnEmpty;
//		default:
//			break;
//		}	
//		return Operation::Undefined;
//	default:
//		break;
//	}
//}

TopoDS_Shape NBooleanFactory::Union(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance)
{
	//Unioning two solids must return one or more solids unless left and right are both empty shapes
	if (IsEmpty(left) && IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Union two empty solids. Result is an empty solid");
		return _emptySolid;
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
		bop.SetFuzzyValue(fuzzyTolerance);

		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();	
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/
		
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Failed to union solids");
	return _emptySolid; //return empty so we fire a managed exception
}

TopoDS_Shape NBooleanFactory::Union(const TopTools_ListOfShape& shapes, double fuzzyTolerance)
{
	if (shapes.Size() == 0)
	{
		pLoggingService->LogWarning("Attempt to Union  a empty shape. Result is an empty shape");
		return TopoDS_Shape();
	}
	if (shapes.Size()<2) 
	{
		pLoggingService->LogWarning("Attempt to Union  a single shapes. Result is the same shape");
		return shapes.First();
	}
	
	//try and union
	try
	{
		BRepAlgoAPI_BooleanOperation bop;

		TopTools_ListOfShape arguments;
		TopTools_ListOfShape tools;
		auto& it = shapes.cbegin(); //take the first one
		tools.Append(*it);	
		for (++it; it != shapes.cend(); ++it)
		{
			arguments.Append(*it);
			
		}		
		bop.SetArguments(arguments);
		bop.SetTools(tools);
		bop.SetOperation(BOPAlgo_Operation::BOPAlgo_FUSE);
		bop.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		bop.SetNonDestructive(true);
		bop.SetFuzzyValue(fuzzyTolerance);
		//bop.SetGlue(BOPAlgo_GlueEnum::BOPAlgo_GlueFull);
		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/

	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Failed to union solids");
	return _emptySolid; //return empty so we fire a managed exception
}

TopoDS_Shape NBooleanFactory::Cut(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance)
{
	//Cutting two solids must return one or more solids unless left and right are both empty shapes
	//or the cut removes all content
	if (IsEmpty(left) && IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Cut two empty solids. Result is an empty solid");
		return _emptySolid;
	}
	if (IsEmpty(right))
	{
		pLoggingService->LogWarning("Attempt to Cut two solids, the right one is empty. Result is the left solid");
		return left;
	}
	if (IsEmpty(left))
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
		bop.SetFuzzyValue(fuzzyTolerance);

		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/

	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Failed to cut solids");
	return _emptySolid; //return empty so we fire a managed exception
}

TopoDS_Shape NBooleanFactory::Intersect(const TopoDS_Shape& left, const TopoDS_Shape& right, double fuzzyTolerance)
{
	//Intersection of two solids can  return one or no solids if left or right are empty shapes
	//then there can be no intersection
	if (IsEmpty(left) || IsEmpty(right))
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
		bop.SetFuzzyValue(fuzzyTolerance);

		bop.Build();
		if (bop.IsDone()) //work out what to do in this situation
		{
			return bop.Shape();
		}
		/*Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(timeout);
		aBOP.SetProgressIndicator(pi);*/

	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Failed to intersect solids");
	return _emptySolid; //return empty so we fire a managed exception
}
