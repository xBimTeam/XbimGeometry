#include "LoggingService.h"
#pragma region Unmanaged
#pragma managed(push, off)
WriteLog LoggerFunc;

void LoggingServiceNative::SetLogger(WriteLog lFunc)
{
		LoggerFunc = lFunc;
		LogInformation("Native Logging Initialised");
}

void LoggingServiceNative::LogCritical(const char* logMsg) { LoggerFunc(5, logMsg); };

void LoggingServiceNative::LogError(const char* logMsg) { LoggerFunc(4, logMsg); };

void LoggingServiceNative::LogWarning(const char* logMsg) { LoggerFunc(3, logMsg); };

void LoggingServiceNative::LogInformation(const char* logMsg) { LoggerFunc(2, logMsg); };

void LoggingServiceNative::LogDebug(const char* logMsg) { LoggerFunc(1, logMsg); };


#pragma endregion

#pragma region Managed
#pragma managed(pop)
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			void LoggingService::LogCritical(String^ logMsg) { LoggerExtensions::LogCritical(_logger, logMsg); };

			void LoggingService::LogError(String^ logMsg) { LoggerExtensions::LogError(_logger, logMsg); };

			void LoggingService::LogWarning(String^ logMsg) { LoggerExtensions::LogWarning(_logger, logMsg); };

			void LoggingService::LogInformation(String^ logMsg) { LoggerExtensions::LogInformation(_logger, logMsg); };

			void LoggingService::LogDebug(String^ logMsg) { LoggerExtensions::LogDebug(_logger, logMsg); };

		}
	}
}
#pragma endregion