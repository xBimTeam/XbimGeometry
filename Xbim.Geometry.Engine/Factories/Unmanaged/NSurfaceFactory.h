#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Edge.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
class NSurfaceFactory : public NFactoryBase
{

public:


	Handle(Geom_Plane) BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ);
	Handle(Geom_Plane) BuildPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& refDir);
	Handle(Geom_CylindricalSurface) BuildCylindricalSurface(double radius);
	Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(Handle(Geom_Curve) sweptCurve, const gp_Vec& sweepDirection);
	Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(const TopoDS_Edge& sweptEdge, const gp_Vec& sweepDirection, bool hasRevitBSplineIssue);
};

