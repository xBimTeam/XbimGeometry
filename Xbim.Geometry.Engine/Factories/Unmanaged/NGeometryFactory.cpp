#include "NGeometryFactory.h"

bool NGeometryFactory::ToLocation(gp_Pnt2d location, gp_XY xDirection, TopLoc_Location& topLoc)
{
	try
	{
		//Direction is not created if it has null magnitude					
		static double res2 = gp::Resolution() * gp::Resolution();
		double aMagnitude = xDirection.SquareModulus();
		if (aMagnitude < res2)
			xDirection = gp_XY(1, 0); //reset to default
		gp_Ax2d axis2d(location, xDirection);
		gp_Trsf2d trsf;
		trsf.SetTransformation(axis2d);
		trsf.Invert();
		topLoc = TopLoc_Location(trsf);
		return true;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return false;
}



bool NGeometryFactory::BuildDirection2d(double x, double y, gp_Vec2d& dir2d)
{
	try
	{
		dir2d.SetXY(gp_XY(x, y)); //this throws an exception if the vector is invalid
		return true;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);

	}
	return false;
}
bool NGeometryFactory::BuildDirection3d(double x, double y, double z, gp_Vec& dir2d)
{
	try
	{
		dir2d.SetXYZ(gp_XYZ(x, y, z)); //this throws an exception if the vector is invalid
		return true;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);

	}
	return false;
}

bool NGeometryFactory::BuildAxis2Placement3d(const gp_Pnt& location, const gp_Vec& axis, const gp_Vec& refDir, gp_Ax2& ax2)
{
	try 
	{
		ax2 = gp_Ax2(location, axis, refDir);
		return true;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);

	}
	return false;
}

bool NGeometryFactory::BuildAxis2Placement2d(const gp_Pnt2d& location,  const gp_Vec2d& refDir, gp_Ax22d& ax2)
{
	try
	{
		ax2 = gp_Ax22d(location, refDir);
		return true;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);

	}
	return false;
}
