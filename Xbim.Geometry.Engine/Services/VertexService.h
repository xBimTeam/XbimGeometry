#pragma once
#include "../BRep/XbimVertex.h"
#include "../Services/Native/BRepService.h"
using namespace System;
using namespace Xbim::Ifc4::Interfaces;
using namespace System::Runtime::InteropServices;
using namespace Microsoft::Extensions::Logging;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public delegate void LogDelegate(int logLevel, String^ str);
			public ref class VertexService
			{
			private: 
				ILogger^ Logger;  
				BRepService* pBRepService;
				LoggingService* pLoggingService;
				LogDelegate^ LogWriter;
				GCHandle gchLogWriter;
			protected:
				void Log(int logLevel, String^ msg);
			public:
				//destructor
				~VertexService()
				{
					Logger = nullptr;
					LogWriter = nullptr;
					this->!VertexService();
				};
				//finalizer release unamaged objects
				!VertexService()
				{
					gchLogWriter.Free();
					if (pBRepService != nullptr) delete pBRepService;
					pBRepService = nullptr;
					if(pLoggingService !=nullptr) delete pLoggingService;
					pLoggingService = nullptr;
					
					

				};
				VertexService(ILogger^ logger);				
				
				XbimVertex^ Build(double x, double y, double z, double tolerance);

			};
		}
	}
}
