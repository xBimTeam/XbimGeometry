#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"
#include <Standard_Failure.hxx>

class NFactoryBase abstract
{
protected:
	NLoggingService* pLoggingService;
public:
	~NFactoryBase()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	//this class will delete the point to the service
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	void LogStandardFailure(const Standard_Failure& e, char* additionalMessage);
	void LogStandardFailure(const Standard_Failure& e);
};

