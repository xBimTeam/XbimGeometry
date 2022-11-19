#include "NGeometryFactory.h"

TopLoc_Location NGeometryFactory::ToLocation(gp_Pnt2d location, gp_XY xDirection)
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
		return TopLoc_Location(trsf);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	return TopLoc_Location();
}

gp_Dir2d NGeometryFactory::BuildDirection2d(double x, double y, bool& isValid)
{
	try
	{
		isValid = true;
		return gp_Dir2d(x,y);
	}
	catch (const Standard_Failure& e)
	{
		isValid = false;
		LogStandardFailure(e);
	}
	return gp_Dir2d();
}
