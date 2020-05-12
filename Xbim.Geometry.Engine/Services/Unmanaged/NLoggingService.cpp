#include "NLoggingService.h"

WriteLog LoggerFunc;

void NLoggingService::SetLogger(WriteLog lFunc)
{
	LoggerFunc = lFunc;
	LoggerFunc(2, "Loading Native Logging Function");
	LogInformation("Native Logging Initialised");
}

void NLoggingService::LogCritical(const char* logMsg) 
{ LoggerFunc(5, logMsg); };

void NLoggingService::LogError(const char* logMsg)
{
	LoggerFunc(4, logMsg);
};

void NLoggingService::LogWarning(const char* logMsg) 
{ LoggerFunc(3, logMsg); };

void NLoggingService::LogInformation(const char* logMsg) 
{ LoggerFunc(2, logMsg); };

void NLoggingService::LogDebug(const char* logMsg) 
{ LoggerFunc(1, logMsg); };

