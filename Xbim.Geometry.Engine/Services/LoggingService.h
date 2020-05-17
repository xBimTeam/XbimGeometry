#pragma once
#pragma warning( disable : 4691 )
#include "../XbimHandle.h"
#include "../Services/Unmanaged/NLoggingService.h"
using namespace Microsoft::Extensions::Hosting;
using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace System::Threading::Tasks;
using namespace System::Threading;
using namespace System;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{

			public delegate void LogDelegate(int logLevel, String^ str);

			public ref class LoggingService : XbimHandle<NLoggingService>, public IXLoggingService
			{
			private:
				ILogger^ _logger;
				IHostApplicationLifetime^ _appLifetime;
				LogDelegate^ LogWriter;
				GCHandle gchLogWriter;

			public:
				LoggingService(ILogger<LoggingService^>^ logger) : XbimHandle(new NLoggingService())
				{
					_logger = logger;					
					LogWriter = gcnew LogDelegate(this, &LoggingService::Log);
					gchLogWriter = GCHandle::Alloc(LogWriter);
					IntPtr ip = Marshal::GetFunctionPointerForDelegate(LogWriter);
					this->Ptr()->SetLogger(static_cast<WriteLog>(ip.ToPointer()));				
				};
				bool ReleaseHandle() override
				{
					gchLogWriter.Free();
					return XbimHandle::ReleaseHandle();
				}
				virtual property ILogger^ Logger {ILogger^  get() { return _logger; }};
				virtual void LogCritical(String^ logMsg);
				virtual void LogError(String^ logMsg);
				virtual void LogWarning(String^ logMsg);
				virtual void LogInformation(String^ logMsg);
				virtual void LogDebug(String^ logMsg);
				virtual operator NLoggingService* ()  { return this->Ptr(); }
				virtual property IntPtr LogDelegatePtr {IntPtr get() { return Marshal::GetFunctionPointerForDelegate(LogWriter); }};
				

				void Log(int logLevel, String^ msg)
				{
					switch (logLevel)
					{
					case 5:
					case 4:
						
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