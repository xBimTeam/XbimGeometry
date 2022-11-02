#pragma once
#include <Geom_Plane.hxx>

#include "../../Services/Unmanaged/NLoggingService.h"
class NSurfaceFactory
{
private:
	NLoggingService* pLoggingService;

public:
	NSurfaceFactory()
	{
		pLoggingService = nullptr;
	};
	~NSurfaceFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };

	Handle(Geom_Plane) BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ);
};

