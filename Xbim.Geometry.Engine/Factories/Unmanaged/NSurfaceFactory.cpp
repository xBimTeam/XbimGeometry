#include "NSurfaceFactory.h"
#include <BRep_Tool.hxx>
Handle(Geom_Plane) NSurfaceFactory::BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ)
{
	try
	{
		return BuildPlane(gp_Pnt(originX, originY, originZ), gp_Dir(normalX, normalY, normalZ)); //normal may throw an exception
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_Plane)();
	}
}

Handle(Geom_Plane) NSurfaceFactory::BuildPlane(const gp_Pnt& origin, const gp_Dir& normal)
{
	try
	{

		//this will throw an exception if the direction is 0
		return new Geom_Plane(origin, normal);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_Plane)();
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
Handle(Geom_SurfaceOfLinearExtrusion) NSurfaceFactory::BuildSurfaceOfLinearExtrusion(const TopoDS_Edge& sweptEdge, const gp_Vec& sweepDirection)
{
	try
	{
		TopLoc_Location loc;
		double start = 0, end = 0;
		Handle(Geom_Curve) sweptCurve = BRep_Tool::Curve(sweptEdge, loc, start, end);
		Handle(Geom_SurfaceOfLinearExtrusion) surface = new Geom_SurfaceOfLinearExtrusion(sweptCurve, sweepDirection);
		if(!loc.IsIdentity()) surface->Transform(loc.Transformation());
		return surface;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_SurfaceOfLinearExtrusion)();
	}
}
