#pragma once

#include "../XbimHandle.h"
#include "Unmanaged/NProjectionService.h"
#include "LoggingService.h"
#include "../BRep/XPolyLoop2d.h"
#include "../BRep/XCompound.h"
using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class ProjectionService : XbimHandle<NProjectionService>, IXProjectionService
			{
			private:
				IXModelGeometryService^ _modelService;
			public:
				ProjectionService(IXModelGeometryService^ modelService) : XbimHandle(new NProjectionService())
				{
					
					_modelService = modelService;
				}
				virtual property IXModelGeometryService^ ModelGeometryService {IXModelGeometryService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _modelService->LoggingService; }};
				virtual IXFootprint^ CreateFootprint(IXShape^ shape, double linearDeflection, double angularDeflection);
				virtual IXFootprint^ CreateFootprint(IXShape^ shape);
				virtual IXCompound^ GetOutline(IXShape^ shape);
				virtual IEnumerable<IXFace^>^ CreateSection(IXShape^ shape, IXPlane^ cutPlane); 
			};
		}
	}
}
