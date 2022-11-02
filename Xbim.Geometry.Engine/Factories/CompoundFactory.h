#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include "./Unmanaged/NCompoundFactory.h"
#include "../Services/LoggingService.h"
#include "GeometryProcedures.h"
#include "CurveFactory.h"
#include "WireFactory.h"
#include "FaceFactory.h"
#include "ShellFactory.h"
#include "ShapeFactory.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class CompoundFactory : XbimHandle<NCompoundFactory>, IXCompoundFactory
			{
				IXLoggingService^ _loggerService;
				IXModelService^ _modelService;
			public:
				CompoundFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NCompoundFactory())
				{
					_loggerService = loggingService;
					_modelService = modelService;
				}
				virtual IXCompound^ CreateEmpty();
				virtual IXCompound^ CreateFrom(IEnumerable<IXShape^>^shapes);
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};
			};
		}
	}
}