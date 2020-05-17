#pragma once
#include "../XbimHandle.h"
#include "Unmanaged/NShapeService.h"
#include "LoggingService.h"
#include <TopoDS_Shape.hxx>

using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;
using namespace Microsoft::Extensions::Hosting;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class ShapeService : XbimHandle<NShapeService>, IXShapeService
			{

			private:
				IXLoggingService^ _logger;
				IHostApplicationLifetime^ _appLifetime;	

			public:
				ShapeService(IXLoggingService^ logger) : XbimHandle(new NShapeService())
				{
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(logger->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
					_logger = logger;					
				};
				

				virtual IXShape^ UnifyDomain(IXShape^ toFix);
				TopoDS_Shape UnifyDomain(const TopoDS_Shape& toFix);
				TopoDS_Solid ShapeService::UnifyDomain(const TopoDS_Solid& toFix);

			};
		}
	}
}

