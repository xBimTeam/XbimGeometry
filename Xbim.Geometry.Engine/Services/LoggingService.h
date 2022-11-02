#pragma once
#pragma warning( disable : 4691 )
#include "../XbimHandle.h"
#include "../Services/Unmanaged/NLoggingService.h"
//using namespace Microsoft::Extensions::Hosting;
using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace System::Threading::Tasks;
using namespace System::Threading;

using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{

			public delegate void LogDelegate(int logLevel, System::String^ str);

			public ref class LoggingService : XbimHandle<NLoggingService>, public IXLoggingService
			{
			private:
				ILogger^ _logger;
				//IHostApplicationLifetime^ _appLifetime;
				LogDelegate^ LogWriter;
				GCHandle gchLogWriter;

			public:
				LoggingService(ILogger<LoggingService^>^ logger) : LoggingService((ILogger^)logger)
				{
				}

				LoggingService(ILogger^ logger) : XbimHandle(new NLoggingService())
				{
					_logger = logger;					
					LogWriter = gcnew LogDelegate(this, &LoggingService::Log);
					gchLogWriter = GCHandle::Alloc(LogWriter);
					//LogInformation("Native Logger handle obtained");
					System::IntPtr ip = Marshal::GetFunctionPointerForDelegate(LogWriter);
					this->Ptr()->SetLogger(static_cast<WriteLog>(ip.ToPointer()));
					
				};
				bool ReleaseHandle() override
				{
					if (gchLogWriter.IsAllocated)
					{
						gchLogWriter.Free();
						//LogInformation("Native Logger handle released");
					}
					else
						LogInformation("Duplicate call to release Native Logger");
					return XbimHandle::ReleaseHandle();
				}
				virtual property ILogger^ Logger {ILogger^  get() { return _logger; }};
				virtual void LogCritical(System::String^ logMsg);
				virtual void LogError(System::String^ logMsg);
				virtual void LogWarning(System::String^ logMsg);
				virtual void LogInformation(System::String^ logMsg);
				virtual void LogDebug(System::String^ logMsg);
				virtual operator NLoggingService* ()  { return this->Ptr(); }
				virtual property System::IntPtr LogDelegatePtr {System::IntPtr get() { return Marshal::GetFunctionPointerForDelegate(LogWriter); }};
				

				void Log(int logLevel, System::String^ msg)
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