#pragma once
#include "../Services/Unmanaged/NLoggingService.h"
#include "Standard_Failure.hxx"


class CanLogBase
{
public:
	void SetLogger(WriteLog lFunc)
	{
		NLoggingService* logService = new NLoggingService();
		logService->SetLogger(lFunc);
		loggingService = logService;
	};
protected:
	void LogStandardFailure(const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		loggingService->LogWarning(strm.str().c_str());
	};

	void LogStandardFailure(const Standard_Failure& e, const char* additionalMessage)
	{
		std::stringstream strm;
		strm << additionalMessage << std::endl;
		e.Print(strm);
		loggingService->LogWarning(strm.str().c_str());
	};

	void LogDebug(const char* message) {
		std::stringstream strm;
		strm << message << std::endl;
		loggingService->LogDebug(strm.str().c_str());
	}

	void LogInfo(const char* message) {
		std::stringstream strm;
		strm << message << std::endl;
		loggingService->LogInformation(strm.str().c_str());
	}

	void LogWarning(const char* message) {
		std::stringstream strm;
		strm << message << std::endl;
		loggingService->LogWarning(strm.str().c_str());
	}

	void LogError(const char* message) {
		std::stringstream strm;
		strm << message << std::endl;
		loggingService->LogError(strm.str().c_str());
	}
private:
	NLoggingService* loggingService;

};