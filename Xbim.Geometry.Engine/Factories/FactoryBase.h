#pragma once
#include "../XbimHandle.h"
#include "../Services/ModelService.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Abstractions;

#define EXEC_NATIVE Ptr()
#define CURVE_FACTORY _modelService->GetCurveFactory()
#define GEOMETRY_FACTORY _modelService->GetGeometryFactory()
#define LOGGING_SERVICE _modelService->LoggingService

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ref class ModelService;
		}
	}
}
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			template <class T>
			public ref class FactoryBase : XbimHandle<T>, IXModelScoped
			{
			protected:
				Xbim::Geometry::Services::ModelService^ _modelService;
				void RaiseGeometryFactoryException(System::String^ message);
				void RaiseGeometryFactoryException(System::String^ message, System::Exception^ innerException);
				void RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity);
				void RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity, System::Exception^ innerException);

			public:

				FactoryBase(Xbim::Geometry::Services::ModelService^ modelService, T* nativeFactory);
				virtual property IXModelService^ ModelService {IXModelService^ get(); }
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get(); }
			};


		}
	}
}

