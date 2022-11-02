#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Edge.hxx>
#include "../Services/LoggingService.h"
#include "Unmanaged/NEdgeFactory.h"

#include "../BRep/XEdge.h"
#include "CurveFactory.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class EdgeFactory : XbimHandle<NEdgeFactory>, IXEdgeFactory
			{
			private:
				IXLoggingService^ _loggerService;
				ILogger^ _logger;
				IXModelService^ _modelService;
				//The distance between two points at which they are determined to be equal points
				
				IXCurveFactory^ _curveFactory;
				
			public:
				EdgeFactory(IXLoggingService^ loggingService, IXModelService^ modelService, IXCurveFactory^ curveFactory) : XbimHandle(new NEdgeFactory())
				{
					_loggerService = loggingService;
					_logger = _loggerService->Logger;
					_modelService = modelService;
					_curveFactory = curveFactory;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(_loggerService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
					
				}
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};
				virtual IXEdge^ BuildEdge(IXPoint^ start, IXPoint^ end);
				virtual IXEdge^ Build(IIfcCurve^ curve);
				virtual IXEdge^ BuildEdge(IXCurve^ curve);
				
				TopoDS_Edge BuildCurve(IIfcCurve^ curve);
			};
		}
	}
}

