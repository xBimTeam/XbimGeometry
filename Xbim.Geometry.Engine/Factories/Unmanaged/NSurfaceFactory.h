#pragma once
#include "NFactoryBase.h"
#include <Geom_Plane.hxx>

class NSurfaceFactory : NFactoryBase
{

public:


	Handle(Geom_Plane) BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ);
	Handle(Geom_Plane) BuildPlane(gp_Pnt origin, gp_Dir normal);
};

