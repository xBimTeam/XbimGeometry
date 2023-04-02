#include "NSurfaceFactory.h"
#include <BRep_Tool.hxx>
Handle(Geom_Plane) NSurfaceFactory::BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ)
{
	try
	{
		return new Geom_Plane(gp_Pnt(originX, originY, originZ), gp_Dir(normalX, normalY, normalZ)); //normal may throw an exception
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_Plane)();
	}
}

Handle(Geom_Plane) NSurfaceFactory::BuildPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& refDir)
{
	try
	{
		gp_Ax3 ax3(origin,normal,refDir);
		//this will throw an exception if the direction is 0
		return new Geom_Plane(ax3);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_Plane)();
	}
}

Handle(Geom_CylindricalSurface) NSurfaceFactory::BuildCylindricalSurface(double radius)
{
	try
	{
		return new Geom_CylindricalSurface(gp::XOY(), radius);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_CylindricalSurface)();
	}
}

Handle(Geom_SurfaceOfLinearExtrusion) NSurfaceFactory::BuildSurfaceOfLinearExtrusion(Handle(Geom_Curve) sweptCurve, const gp_Vec& sweepDirection)
{

	try
	{
		return new Geom_SurfaceOfLinearExtrusion(sweptCurve, sweepDirection);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_SurfaceOfLinearExtrusion)();
	}
}

/// <summary>
/// This creates a surface from the curve within the edge and transforms the surface to the location defined in the edge
/// </summary>
/// <param name="sweptEdge"></param>
/// <param name="sweepDirection"></param>
/// <returns></returns>
Handle(Geom_SurfaceOfLinearExtrusion) NSurfaceFactory::BuildSurfaceOfLinearExtrusion(const TopoDS_Edge& sweptEdge, const gp_Vec& sweepDirection, bool hasRevitBSplineIssue)
{
	try
	{
		TopLoc_Location loc;
		double start = 0, end = 0;
		Handle(Geom_Curve) sweptCurve = BRep_Tool::Curve(sweptEdge, loc, start, end);
		//to get an accurate ruled surface we should use the trim parameters to define the bounds
		//however, when the surface is used in aspects such as IfcSurfaceCurveSweptAreaSolid then some BIM tools redefine the bounds to be larger than the ruled surface
		//this will throw an error, should we need rigidly bounded surafaces it is suggested these are built as Faces with appropriate Edges
		//To create a ruled surface use the line below and pass the TrimmedCurve to the Geom_SurfaceOfLinearExtrusion ctor
		//Handle(Geom_TrimmedCurve) trimmedCurve = new Geom_TrimmedCurve(sweptCurve, start, end);
		Handle(Geom_SurfaceOfLinearExtrusion) surface = new Geom_SurfaceOfLinearExtrusion(sweptCurve, sweepDirection);
		if(!hasRevitBSplineIssue && !loc.IsIdentity())
			surface->Transform(loc.Transformation());
		return surface;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_SurfaceOfLinearExtrusion)();
	}
}
