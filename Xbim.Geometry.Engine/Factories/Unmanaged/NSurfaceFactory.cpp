#include "NSurfaceFactory.h"

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

Handle(Geom_Plane) NSurfaceFactory::BuildPlane(gp_Pnt origin, gp_Dir normal)
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
