#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Shell.hxx>
#include "./Unmanaged/NShellFactory.h"
#include "../Services/LoggingService.h"

using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ShellFactory : XbimHandle<NShellFactory>, IXShellFactory
			{
				IXLoggingService^ loggerService;
				IXModelService^ _modelService;
			public:
				ShellFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NShellFactory())
				{
					loggerService = loggingService;		
					_modelService = modelService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return loggerService; }};
				TopoDS_Shell BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet);
			};
		}
	}

};

