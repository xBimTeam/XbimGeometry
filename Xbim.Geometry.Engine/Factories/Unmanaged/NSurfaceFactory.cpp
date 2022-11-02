#include "NSurfaceFactory.h"

Handle(Geom_Plane) NSurfaceFactory::BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ)
{
	
	try
	{
		//this will throw an exception if the direction is 0
		return new Geom_Plane(gp_Pnt(originX, originY, originZ), gp_Dir(normalX, normalY, normalZ));
	}
	catch (const Standard_Failure& sf)
	{
		std::stringstream strm;
		sf.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
		return nullptr;
	}
}
