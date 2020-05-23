#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"
#include <gp_XYZ.hxx>


#include <vector>

class NGeomProcFactory
{
private:
	NLoggingService* pLoggingService;
public:
	NGeomProcFactory()
	{
		pLoggingService = nullptr;
	};

	~NGeomProcFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	//this class will delete the point to the service
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	
};

