#include "NLoggingService.h"
#include <iostream>

WriteLog LoggerFunc = nullptr;

void NLoggingService::SetLogger(WriteLog lFunc)
{
	LoggerFunc = lFunc;
	//LogInformation("Native Logger attached");
}

WriteLog NLoggingService::GetLogger()
{
	return LoggerFunc;
}

void NLoggingService::LogCritical(const char* logMsg)
{
	if (LoggerFunc != nullptr)
		LoggerFunc(5, logMsg);
	else
		std::cout << "Critical: " << logMsg;

};

void NLoggingService::LogError(const char* logMsg)
{
	if (LoggerFunc != nullptr)
		LoggerFunc(4, logMsg);
	else
		std::cout << "Error: " << logMsg;
};

void NLoggingService::LogWarning(const char* logMsg)
{
	if (LoggerFunc != nullptr)
		LoggerFunc(3, logMsg);
	else
		std::cout << "Warning: " << logMsg;
};

void NLoggingService::LogInformation(const char* logMsg)
{
	if (LoggerFunc != nullptr)
		LoggerFunc(2, logMsg);
	else
		std::cout << "Information: " << logMsg;
};

void NLoggingService::LogDebug(const char* logMsg)
{
	if (LoggerFunc != nullptr)
		LoggerFunc(1, logMsg);
	else
		std::cout << "Debug: " << logMsg;
};

