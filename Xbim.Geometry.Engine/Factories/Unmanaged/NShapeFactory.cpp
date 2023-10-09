#include "NShapeFactory.h"
#include <BRepCheck_Shell.hxx>
#include <BRep_Builder.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

#include <BRepMesh_IncrementalMesh.hxx>
#include <BOPAlgo_BOP.hxx>
#include "../../Services/Unmanaged/NProgressMonitor.h"
#include <BRepTools.hxx>
#include <Standard_Handle.hxx>






TopoDS_Shape NShapeFactory::UnifyDomain(const TopoDS_Shape& toFix, double linearTolerance, double angularTolerance)
{
	try
	{
		ShapeUpgrade_UnifySameDomain unifier(toFix);
		unifier.SetLinearTolerance(linearTolerance);
		unifier.SetAngularTolerance(angularTolerance);
		unifier.Build();
		return unifier.Shape();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogWarning("Failed to unify shape domain");
	return toFix;
}

TopoDS_Solid NShapeFactory::UnifyDomain(const TopoDS_Solid& toFix, double linearTolerance, double angularTolerance)
{
	try
	{
		ShapeUpgrade_UnifySameDomain unifier(toFix);
		unifier.SetLinearTolerance(linearTolerance);
		unifier.SetAngularTolerance(angularTolerance);
		unifier.Build();
		if (unifier.Shape().ShapeType() == TopAbs_SOLID) //if it doesn't something has gone wrong, most likely the orginal solid is badly formed
			return TopoDS::Solid(unifier.Shape());
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogWarning("Failed to unify domain");
	return toFix;
}

TopoDS_Shape NShapeFactory::Convert(const char* brepString)
{
	try
	{
		TopoDS_Shape result;
		BRep_Builder builder;
		std::istringstream iss(brepString);
		BRepTools::Read(result, iss, builder);
		switch (result.ShapeType())
		{
		case TopAbs_VERTEX:
		case TopAbs_EDGE:
		case TopAbs_WIRE:
		case TopAbs_FACE:
		case TopAbs_SHELL:
		case TopAbs_SOLID:
		case TopAbs_COMPOUND:
			return result;
		default:
			pLoggingService->LogError("Unsupported Shape Type");
		}
	}
	catch (const Standard_Failure& e)
	{
		pLoggingService->LogError("Failed convert Brep string");
		LogStandardFailure(e);
	}
	
	return TopoDS_Shape();
}

const char* NShapeFactory::Convert(TopoDS_Shape shape)
{
	std::ostringstream oss;

	BRepTools::Write(shape, oss);


	return nullptr;
}

bool NShapeFactory::Triangulate(const TopoDS_Shape& aShape, const IMeshTools_Parameters& meshParams)
{
	try
	{

		BRepMesh_IncrementalMesh aMesher(aShape, meshParams.Deflection, meshParams.Relative, meshParams.Angle, meshParams.InParallel);
		const Standard_Integer aStatus = aMesher.GetStatusFlags();
		return !aStatus;
	}
	catch (const Standard_Failure&)
	{
		//pLoggingService->LogWarning(e.GetMessageString());
	}
	//pLoggingService->LogError("Failed triangulate shape");
	return false;
}

void NShapeFactory::CheckSelfIntersection(const BOPAlgo_BOP& booleanOperation)
{
	const opencascade::handle<Message_Report> & report =  booleanOperation.GetReport();
	const Message_ListOfAlert& alerts =  report->GetAlerts(Message_Warning);
	for (Message_ListOfAlert::Iterator it(alerts); it.More(); it.Next())
	{
		const Handle(Message_Alert)& alert = it.Value();
		const TCollection_AsciiString& alertText = alert->GetMessageKey();
		if(alertText == "BOPAlgo_AlertAcquiredSelfIntersection")
		{
			Standard_Failure::Raise("Self intersection detected");
		}
	}
}

std::pair<bool, TopoDS_Shape> NShapeFactory::Cut(const TopoDS_Shape& body, const TopTools_ListOfShape& subtractions, double minumGap)
{
	try
	{
		BOPAlgo_BOP aBOP;
		aBOP.AddArgument(body);
		aBOP.SetTools(subtractions);
		aBOP.SetOperation(BOPAlgo_CUT);
		aBOP.SetRunParallel(false);
		//aBOP.SetCheckInverted(false);
		aBOP.SetNonDestructive(true);
		aBOP.SetFuzzyValue(minumGap);
		NProgressMonitor pi(_timeout);
		//aBOP.SetGlue(BOPAlgo_GlueEnum::BOPAlgo_GlueFull);
		TopoDS_Shape aR;
		aBOP.Perform(pi);
		if (aBOP.HasErrors())
		{
			std::stringstream errMsg;
			errMsg << std::endl << "Error cutting shape" << std::endl;
			aBOP.DumpErrors(errMsg);
			Standard_Failure::Raise(errMsg);
		}
		if (aBOP.HasWarnings())
		{
			std::stringstream warningMsg;
			warningMsg << std::endl << "Warnings cutting shape" << std::endl;
			aBOP.DumpWarnings(warningMsg);
			pLoggingService->LogWarning(warningMsg.str().c_str());
			CheckSelfIntersection(aBOP);
		}
		return std::pair<bool, TopoDS_Shape>(true, aBOP.Shape());
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return std::pair<bool, TopoDS_Shape>(false, TopoDS_Shape());
}

std::pair<bool, TopoDS_Shape> NShapeFactory::Cut(const TopoDS_Shape& body, const TopoDS_Shape& subtraction, double minumGap)
{
	TopTools_ListOfShape subtractions;
	subtractions.Append(subtraction);
	return Cut(body, subtractions, minumGap);
}

std::pair<bool, TopoDS_Shape> NShapeFactory::Union(const TopoDS_Shape& body, const TopTools_ListOfShape& additions, double minumGap)
{
	try
	{
		BOPAlgo_BOP aBOP;
		aBOP.AddArgument(body);
		aBOP.SetTools(additions);
		aBOP.SetOperation(BOPAlgo_FUSE);
		aBOP.SetRunParallel(false);
		//aBOP.SetCheckInverted(true);
		aBOP.SetNonDestructive(true);
		aBOP.SetFuzzyValue(minumGap);
		NProgressMonitor pi(_timeout);

		TopoDS_Shape aR;
		aBOP.Perform(pi);
		if (aBOP.HasErrors())
		{
			std::stringstream errMsg;
			errMsg << std::endl << "Error projecting shape" << std::endl;
			aBOP.DumpErrors(errMsg);
			Standard_Failure::Raise(errMsg);
		}
		if (aBOP.HasWarnings())
		{
			std::stringstream warningMsg;
			warningMsg << std::endl << "Warnings projecting shape" << std::endl;
			aBOP.DumpWarnings(warningMsg);
			pLoggingService->LogWarning(warningMsg.str().c_str());
		}
		return std::pair<bool, TopoDS_Shape>(true, aBOP.Shape());
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return std::pair<bool, TopoDS_Shape>(false, TopoDS_Shape());
}

ShapeExtend_Status NShapeFactory::FixFace(const TopoDS_Face& face, TopTools_ListOfShape& result)
{
	ShapeExtend_Status status = ShapeExtend_Status::ShapeExtend_DONE;
	try
	{
		ShapeFix_Shape faceFixer(face);
		bool ok = faceFixer.Perform();
		faceFixer.Status(status);
		if (ok)
		{
			TopoDS_Shape shape = faceFixer.Shape();
			if (shape.ShapeType() == TopAbs_ShapeEnum::TopAbs_FACE)
			{
				result.Append(shape);
				return status;
			}
			else
			{
				for (TopExp_Explorer exp(shape, TopAbs_ShapeEnum::TopAbs_FACE); exp.More(); exp.Next())
					result.Append(exp.Current());
				return status;
			}
		}
		//if here we have failed
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogWarning("Failed to fix face");
	return status;
}

std::pair<bool, TopoDS_Shape> NShapeFactory::Union(const TopoDS_Shape& body, const TopoDS_Shape& addition, double minumGap)
{
	TopTools_ListOfShape additions;
	additions.Append(addition);
	return Union(body, additions, minumGap);
}




