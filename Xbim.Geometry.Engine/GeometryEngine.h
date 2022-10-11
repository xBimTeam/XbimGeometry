#pragma once
#include "../Version 5/XbimGeometryCreator.h"
namespace Xbim
{
	namespace Geometry
	{

		public ref class GeometryEngineV5 : XbimGeometryCreator, IXGeometryEngineV5
		{
		private:
			IXLoggingService^ _loggerService;
			IXModelService^ _modelService;
		public:
			GeometryEngineV5(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimGeometryCreator(modelService)
			{
				_loggerService = loggingService;
				_modelService = modelService;
			};
			virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
			virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};
		};
	}
}
