#include "LoggingService.h"

using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			void LoggingService::LogCritical(String^ logMsg) {
				LoggerExtensions::LogCritical(_logger, logMsg); };

			void LoggingService::LogError(String^ logMsg) { 
				LoggerExtensions::LogError(_logger, logMsg); };

			void LoggingService::LogWarning(String^ logMsg) { 
				LoggerExtensions::LogWarning(_logger, logMsg); };

			void LoggingService::LogInformation(String^ logMsg) {
				LoggerExtensions::LogInformation(_logger, logMsg); };

			void LoggingService::LogDebug(String^ logMsg) { LoggerExtensions::LogDebug(_logger, logMsg); };

		}
	}
}
