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
	NFactoryBase()
	{

	}
	NFactoryBase(WriteLog lFunc)
	{
		SetLogger(lFunc);
	}
	//this class will delete the point to the service
	virtual void SetLogger(WriteLog lFunc)
	{ 
		NLoggingService* logService = new NLoggingService();
		logService->SetLogger(lFunc); 
		pLoggingService = logService; 
	};
	void LogStandardFailure(const Standard_Failure& e, char* additionalMessage);
	void LogStandardFailure(const Standard_Failure& e);
};

