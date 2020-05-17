#include "NShapeService.h"
#include <Precision.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopoDS.hxx>

TopoDS_Shape NShapeService::UnifyDomain(const TopoDS_Shape& toFix)
{
	try
	{
		ShapeUpgrade_UnifySameDomain unifier(toFix);
		//unifier.SetLinearTolerance(linearTolerance);
		//unifier.SetAngularTolerance(angularTolerance);
		unifier.Build();
		return unifier.Shape();
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Failed to unify shape domain");
	return toFix;
}

TopoDS_Solid NShapeService::UnifyDomain(const TopoDS_Solid& toFix)
{
	try
	{
		ShapeUpgrade_UnifySameDomain unifier(toFix);
		//unifier.SetLinearTolerance(linearTolerance);
		//unifier.SetAngularTolerance(angularTolerance);
		unifier.Build();
		if(unifier.Shape().ShapeType()==TopAbs_SOLID) //if it doesn't something has gone wrong, most likely the orginal solid is badly formed
			return TopoDS::Solid(unifier.Shape());
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Failed to unify solid domain");
	return toFix;
}