#pragma once
#include "../XbimHandle.h"
#include "Unmanaged/NSurfaceFactory.h"
#include "GeometryProcedures.h"
#include "CurveFactory.h"
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class SurfaceFactory : XbimHandle<NSurfaceFactory>, IXSurfaceFactory	
			{
			private:
				IXLoggingService^ _loggerService;
				IXModelService^ _modelService;
				GeometryProcedures^ _GeometryProcedures;
				CurveFactory^ _curveFactory;
			public:
				SurfaceFactory(IXLoggingService^ loggingService, IXModelService^ modelService, IXCurveFactory^ curveFactory) : XbimHandle(new NSurfaceFactory())
				{
					_loggerService = loggingService;
					_modelService = modelService;					
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
					_GeometryProcedures = gcnew GeometryProcedures(loggingService, modelService);
					_curveFactory = dynamic_cast<CurveFactory^>(curveFactory);
				}

				virtual IXPlane^ BuildPlane(IXPoint^ origin, IXDirection^ normal);
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};
			};
		}
	}
}

