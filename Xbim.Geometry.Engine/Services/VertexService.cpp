#include "../stdafx.h"
#include "VertexService.h"
#include "Native/BRepService.h"
using namespace Microsoft::Extensions::Logging;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			
			VertexService::VertexService(ILogger^ logger)
			{
				Logger = logger;
				//setup native logging service					
				LogWriter = gcnew LogDelegate(this, &VertexService::Log);
				gchLogWriter = GCHandle::Alloc(LogWriter);
				IntPtr ip = Marshal::GetFunctionPointerForDelegate(LogWriter);
				pLoggingService = new LoggingService(static_cast<WriteLog>(ip.ToPointer()));
				pBRepService = new BRepService(pLoggingService);
			}
			void VertexService::Log(int logLevel, String^ msg)
			{
				switch (logLevel)
				{
				case 5:
				case 4:
					//Logger->Log(LogLevel::Error,gcnew EventId(0), msg,nullptr);
					LoggerExtensions::LogError(Logger, msg);
					break;
				case 3:
					LoggerExtensions::LogWarning(Logger, msg);
					break;
				case 2:
					LoggerExtensions::LogInformation(Logger, msg);
					break;
				case 1:
				default:
					LoggerExtensions::LogDebug(Logger, msg);
					break;
				}
			}

			XbimVertex^ VertexService::Build(double x, double y, double z, double tolerance)
			{
				TopoDS_Vertex vertex;
				OperationStatus status = pBRepService->BuildVertex(x, y, z, tolerance, vertex);
				if (status == Fail)
					throw gcnew Exception("Failed to create vertex");
				else
					return gcnew XbimVertex(vertex);
			}
		}
	}
}

