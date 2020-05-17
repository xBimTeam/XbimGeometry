#include "NProfileFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
TopoDS_Compound NProfileFactory::MakeCompound(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
	TopoDS_Compound compound;
	builder.MakeCompound(compound);
	builder.Add(compound, shape1);
	builder.Add(compound, shape2);
	return compound;
}

TopoDS_Face NProfileFactory::MakeFace(const TopoDS_Wire& wire, Handle(Geom_Surface) surface)
{
	try
	{
		BRepBuilderAPI_MakeFace faceMaker(surface, wire, true);
		if (faceMaker.IsDone())
			return faceMaker.Face();
		else
		{
			BRepBuilderAPI_FaceError error = faceMaker.Error();
			switch (error)
			{
			case BRepBuilderAPI_FaceDone: //should never happen
				pLoggingService->LogInformation("BRepBuilderAPI_FaceDone: ignore");
				break;
			case BRepBuilderAPI_NoFace:
				pLoggingService->LogWarning("BRepBuilderAPI_NoFace");
				break;
			case BRepBuilderAPI_NotPlanar:
				pLoggingService->LogWarning("BRepBuilderAPI_NotPlanar");
				break;
			case BRepBuilderAPI_CurveProjectionFailed:
				pLoggingService->LogWarning("BRepBuilderAPI_CurveProjectionFailed");
				break;
			case BRepBuilderAPI_ParametersOutOfRange:
				pLoggingService->LogWarning("BRepBuilderAPI_ParametersOutOfRange");
				break;
			default:
				break;
			}
		}
	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogError(e.GetMessageString());
	}
	pLoggingService->LogError("Failed to build face");
	return _emptyFace;
}
