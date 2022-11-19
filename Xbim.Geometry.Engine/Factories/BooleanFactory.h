#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include "../Services/LoggingService.h"
#include "./Unmanaged/NBooleanFactory.h"
#include "SolidFactory.h"


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
			public ref class BooleanFactory : XbimHandle<NBooleanFactory>, IXBooleanFactory
			{
			private:
				//GeometryProcedures^ GpFactory;
				SolidFactory^ _solidFactory;
				ShapeFactory^ _shapeFactory;
				IXLoggingService^ _loggerService;
				ILogger^ Logger;
				IXModelService^ _modelService;
							
			public:
				BooleanFactory(ModelService^ modelService) : XbimHandle(new NBooleanFactory())
				{					
					_modelService = modelService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
					
				}
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};
				virtual IXShape^ Build(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildBooleanResult(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildOperand(IIfcBooleanOperand^ boolOp);
				TopoDS_Solid BuildHalfSpace(IIfcHalfSpaceSolid^ halfSpace);
			};
		}
	}
}

