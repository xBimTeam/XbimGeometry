#pragma once

#include "NFactoryBase.h"

#include <gp_XYZ.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Dir2d.hxx>


class NGeometryFactory : NFactoryBase
{
public:
	
	TopLoc_Location ToLocation(gp_Pnt2d location, gp_XY xDirection);
	gp_Dir2d BuildDirection2d(double x, double y, bool& isValid);
};

