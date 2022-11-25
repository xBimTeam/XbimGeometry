#pragma once

#include "NFactoryBase.h"

#include <TopLoc_Location.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax22d.hxx>

class NGeometryFactory : public NFactoryBase
{
public:

	bool ToLocation(gp_Pnt2d location, gp_XY xDirection, TopLoc_Location& topLoc);
	bool BuildDirection2d(double x, double y, gp_Vec2d& dir2d);
	bool BuildDirection3d(double x, double y, double z, gp_Vec& dir2d);
	bool BuildAxis2Placement3d(const gp_Pnt& location, const gp_Vec& axis, const gp_Vec& refDir, gp_Ax2& ax2);
	bool BuildAxis2Placement2d(const gp_Pnt2d& location,  const gp_Vec2d& refDir, gp_Ax22d& ax2);
};

