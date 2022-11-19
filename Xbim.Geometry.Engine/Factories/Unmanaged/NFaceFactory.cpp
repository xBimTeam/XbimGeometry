#include "NFaceFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepGProp_Face.hxx>

TopoDS_Face NFaceFactory::BuildProfileDef(gp_Pln plane, const TopoDS_Wire& wire)
{
	try
	{
		BRepBuilderAPI_MakeFace faceMaker(plane, wire, true);
		return faceMaker.Face();
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}
	pLoggingService->LogWarning("Could not build face from profile def");
	return TopoDS_Face();
}

gp_Vec NFaceFactory::Normal(const TopoDS_Face& face)
{
	BRepGProp_Face prop(face);
	gp_Pnt centre;
	gp_Vec faceNormal;
	double u1, u2, v1, v2;
	prop.Bounds(u1, u2, v1, v2);
	prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, faceNormal);
	return faceNormal;
}
