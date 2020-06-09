#pragma once
#include "../XbimHandle.h"
#include "./Unmanaged/NFaceFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
#include "WireFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class FaceFactory : XbimHandle<NFaceFactory>
			{
			private:
				IXLoggingService^ LoggerService;
				IXModelService^ ModelService;
				GeomProcFactory^ GPFactory;
				WireFactory^ _wireFactory;
			internal:
				TopoDS_Face BuildProfileDef(IIfcProfileDef^ profileDef);
			public:
				FaceFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NFaceFactory())
				{
					LoggerService = loggingService;
					ModelService = modelService;
					GPFactory = gcnew GeomProcFactory(loggingService, modelService);
					_wireFactory = gcnew WireFactory(loggingService, modelService);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}

			};
		}
	}
}

