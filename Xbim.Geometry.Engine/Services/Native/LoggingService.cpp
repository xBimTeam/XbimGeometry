#include "LoggingService.h"
static WriteLog LoggerFunc;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services {
			namespace Native
			{
				void LoggingService::InitNativeLogging(WriteLog lFunc)
				{
					LoggerFunc = lFunc;
					LoggerFunc(2, -1, "", "Native Logging Initialised");
				};

				void LoggingService::LogCritical(int label, const char* ifcType, const char* logMsg) { LoggerFunc(5, label, ifcType, logMsg); };
				void LoggingService::LogError(int label, const char* ifcType, const char* logMsg) { LoggerFunc(4, label, ifcType, logMsg); };
				void LoggingService::LogWarning(int label, const char* ifcType, const char* logMsg) { LoggerFunc(3, label, ifcType, logMsg); };
				void LoggingService::LogInformation(int label, const char* ifcType, const char* logMsg) { LoggerFunc(2, label, ifcType, logMsg); };
				void LoggingService::LogDebug(int label, const char* ifcType, const char* logMsg) { LoggerFunc(1, label, ifcType, logMsg); };
			}
		}
	}
}