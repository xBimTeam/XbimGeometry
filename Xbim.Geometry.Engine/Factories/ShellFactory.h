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
				
				IXModelService^ _modelService;
			public:
				ShellFactory(ModelService^ modelService) : XbimHandle(new NShellFactory())
				{
						
					_modelService = modelService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _modelService->LoggingService; }};
				TopoDS_Shell BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet);
			};
		}
	}

};

