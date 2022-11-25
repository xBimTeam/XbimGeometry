#pragma once
#include "FactoryBase.h"
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
			public ref class ShellFactory : FactoryBase<NShellFactory>, IXShellFactory
			{
				
				IXModelService^ _modelService;
			public:
				ShellFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NShellFactory()){}
				
				TopoDS_Shell BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet);
			};
		}
	}

};

