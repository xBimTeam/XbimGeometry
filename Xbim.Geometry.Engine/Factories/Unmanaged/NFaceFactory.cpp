#include "NFaceFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepGProp_Face.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopExp_Explorer.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepTools.hxx>
TopoDS_Face NFaceFactory::BuildProfileDef(gp_Pln plane, const TopoDS_Wire& wire)
{
	try
	{
		BRepBuilderAPI_MakeFace faceMaker(plane, wire, true);
		return faceMaker.Face();
	}
	catch (const Standard_Failure& e)
	{
		pLoggingService->LogWarning("Could not build face from profile def");
		LogStandardFailure(e);
	}

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

TopoDS_Face NFaceFactory::BuildFace(Handle(Geom_Surface) surface, double tolerance)
{
	try
	{
		BRepBuilderAPI_MakeFace faceMaker(surface, false);
		if (!faceMaker.IsDone())
			Standard_Failure::Raise("Could not apply surface to face");
		return faceMaker.Face();
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Face specification error");
	}

	return TopoDS_Face();

}
TopoDS_Face NFaceFactory::BuildFace(Handle(Geom_Surface) surface, const TopoDS_Wire& outerLoop, const TopTools_SequenceOfShape& innerLoops, double tolerance)
{
	try
	{
		BRepBuilderAPI_MakeFace faceMaker(surface, outerLoop, false);
		if (!faceMaker.IsDone())
			Standard_Failure::Raise("Could not apply outer bound to face");

		if (innerLoops.Size() > 0) //add any inner bounds
		{
			for (auto&& innerLoop : innerLoops)
			{
				faceMaker.Add(TopoDS::Wire(innerLoop));
				if (!faceMaker.IsDone())
					Standard_Failure::Raise("Could not apply inner bounds to face, they have been ignored");
			}
		}
		Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(surface);


		if (plane.IsNull()) // no need to add pcurves to planar surfaces
		{
			Handle(ShapeFix_Wire) sfw = new ShapeFix_Wire;
			sfw->ClearModes();
			sfw->FixAddPCurveMode() = true;
			sfw->FixSameParameterMode() = true;

			const TopoDS_Face& face = faceMaker.Face();
			for (TopExp_Explorer exp(face, TopAbs_WIRE); exp.More(); exp.Next())
			{
				auto wire = TopoDS::Wire(exp.Current());
				sfw->Init(wire, face, tolerance);
				sfw->FixEdgeCurves();

			}
		}
		return faceMaker.Face();

	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "Face specification error");
	}

	return TopoDS_Face();

}