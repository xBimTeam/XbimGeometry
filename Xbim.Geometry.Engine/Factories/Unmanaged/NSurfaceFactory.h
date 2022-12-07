#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Edge.hxx>
#include <Geom_Plane.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
class NSurfaceFactory : public NFactoryBase
{

public:


	Handle(Geom_Plane) BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ);
	Handle(Geom_Plane) BuildPlane(gp_Pnt origin, gp_Dir normal);
	Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(Handle(Geom_Curve) sweptCurve, gp_Vec sweepDirection);
	Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(const TopoDS_Edge& sweptEdge, gp_Vec sweepDirection);
};

