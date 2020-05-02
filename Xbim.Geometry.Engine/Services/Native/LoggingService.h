#pragma once
typedef void(__stdcall* WriteLog)(int logLevel, int entityLabel, const char* ifcType, const char*  msg);


namespace Xbim
{
	namespace Geometry
	{
		namespace Services {
			namespace Native 
			{
				class LoggingService
				{
				private:

				public:
					static void InitNativeLogging(WriteLog lFunc);

					static void LogCritical(int label, const char* ifcType, const char* logMsg);
					static void LogError(int label, const char* ifcType, const char* logMsg);
					static void LogWarning(int label, const char* ifcType, const char* logMsg);
					static void LogInformation(int label, const char* ifcType, const char* logMsg);
					static void LogDebug(int label, const char* ifcType, const char* logMsg);
				};
			}
		}
	}
}

