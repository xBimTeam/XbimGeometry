#include "NFaceFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>

TopoDS_Face NFaceFactory::BuildProfileDef(gp_Pln plane, const TopoDS_Wire& wire)
{
	try
	{
		BRepBuilderAPI_MakeFace faceMaker(plane, wire, true);
		return faceMaker.Face();
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogWarning(e.GetMessageString());
	}
	pLoggingService->LogWarning("Could not build face from profile def");
	return _emptyFace;
}
