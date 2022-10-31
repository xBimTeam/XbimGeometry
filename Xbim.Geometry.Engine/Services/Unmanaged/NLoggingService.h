#pragma once

typedef void(__stdcall* WriteLog)(int level, const char* msg);

class NLoggingService
{
private:

public:

	NLoggingService() {};

	void SetLogger(WriteLog lFunc);


	void LogCritical(const char* logMsg);

	void LogError(const char* logMsg);

	void LogWarning(const char* logMsg);

	void LogInformation(const char* logMsg);

	void LogDebug(const char* logMsg);
};



