#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include "../Services/LoggingService.h"
#include "./Unmanaged/NBooleanFactory.h"
#include "SolidFactory.h"
#include "../Services/ShapeService.h"
//#include "GeomProcFactory.h"

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
			public ref class BooleanFactory : XbimHandle<NBooleanFactory>
			{
			private:
				//GeomProcFactory^ GpFactory;
				SolidFactory^ _solidFactory;
				ShapeService^ _shapeService;
				IXLoggingService^ LoggerService;
				ILogger^ Logger;
				IXModelService^ ModelService;
							
			public:
				BooleanFactory(IXLoggingService^ loggingService,  IXModelService^ ifcModel) : XbimHandle(new NBooleanFactory())
				{
					//GpFactory = gcnew GeomProcFactory();
					_solidFactory = gcnew SolidFactory(loggingService, ifcModel);
					_shapeService = gcnew ShapeService(loggingService);
					LoggerService = loggingService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
					Ptr()->SetFuzzyTolerance(ifcModel->MinimumGap);
				}
				void SetFuzzyTolerance(double fuzzyTolerance) { Ptr()->SetFuzzyTolerance(fuzzyTolerance); };
				double GetFuzzyTolerance() { return Ptr()->GetFuzzyTolerance(); };
				virtual IXShape^ Build(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildBooleanResult(IIfcBooleanResult^ boolResult);
				TopoDS_Shape BuildOperand(IIfcBooleanOperand^ boolOp);
				TopoDS_Solid BuildHalfSpace(IIfcHalfSpaceSolid^ halfSpace);
			};
		}
	}
}

