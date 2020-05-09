#pragma once
#pragma warning( disable : 4691 )
#include "../XbimHandle.h"
using namespace Microsoft::Extensions::Hosting;
using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace System::Threading::Tasks;
using namespace System::Threading;
using namespace System;

typedef void(__stdcall* WriteLog)(int level, const char* msg);

public class LoggingServiceNative
{
private:

public:

	LoggingServiceNative() {};

	void SetLogger(WriteLog lFunc);


	void LogCritical(const char* logMsg);

	void LogError(const char* logMsg);

	void LogWarning(const char* logMsg);

	void LogInformation(const char* logMsg);

	void LogDebug(const char* logMsg);
};

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{

			public delegate void LogDelegate(int logLevel, String^ str);

			public ref class LoggingService : XbimHandle<LoggingServiceNative>, public IHostedService
			{
			private:
				ILogger^ _logger;
				IHostApplicationLifetime^ _appLifetime;
				LogDelegate^ LogWriter;
				GCHandle gchLogWriter;

			public:
				LoggingService(ILogger<LoggingService^>^ logger, IHostApplicationLifetime^ appLifetime) : XbimHandle(new LoggingServiceNative())
				{
					_logger = logger;
					_appLifetime = appLifetime;
				};
				void LogCritical(String^ logMsg);

				void LogError(String^ logMsg);

				void LogWarning(String^ logMsg);

				void LogInformation(String^ logMsg);

				void LogDebug(String^ logMsg);
				virtual operator LoggingServiceNative* ()  { return this->Ptr(); }

				virtual Task^ StartAsync(System::Threading::CancellationToken cancellationToken)
				{

					LogWriter = gcnew LogDelegate(this, &LoggingService::Log);
					gchLogWriter = GCHandle::Alloc(LogWriter);
					IntPtr ip = Marshal::GetFunctionPointerForDelegate(LogWriter);
					this->Ptr()->SetLogger(static_cast<WriteLog>(ip.ToPointer()));
					return Task::CompletedTask;
				};

				virtual Task^ StopAsync(System::Threading::CancellationToken cancellationToken)
				{
					LogWriter = nullptr;
					gchLogWriter.Free();
					return Task::CompletedTask;
				};

				void Log(int logLevel, String^ msg)
				{
					switch (logLevel)
					{
					case 5:
					case 4:
						//Logger->Log(LogLevel::Error,gcnew EventId(0), msg,nullptr);
						LoggerExtensions::LogError(_logger, msg);
						break;
					case 3:
						LoggerExtensions::LogWarning(_logger, msg);
						break;
					case 2:
						LoggerExtensions::LogInformation(_logger, msg);
						break;
					case 1:
					default:
						LoggerExtensions::LogDebug(_logger, msg);
						break;
					}
				}


			};

		}
	}
}