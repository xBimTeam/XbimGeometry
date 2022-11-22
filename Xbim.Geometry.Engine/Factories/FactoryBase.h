#pragma once
#include "../XbimHandle.h"
#include "../Services/ModelService.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Abstractions;

#define EXEC_NATIVE Ptr()
#define SURFACE_FACTORY _modelService->GetSurfaceFactory()
#define CURVE_FACTORY _modelService->GetCurveFactory()
#define GEOMETRY_FACTORY _modelService->GetGeometryFactory()
#define EDGE_FACTORY _modelService->GetEdgeFactory()
#define WIRE_FACTORY _modelService->GetWireFactory()
#define PROFILE_FACTORY _modelService->GetProfileFactory()

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
				void LogError(System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogError(IPersistEntity^ entity, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogError(IPersistEntity^ entity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogError(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogWarning(System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogWarning(IPersistEntity^ entity, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogWarning(IPersistEntity^ entity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogWarning(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogInformation(System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogInformation(IPersistEntity^ entity, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogInformation(IPersistEntity^ entity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogInformation(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogDebug(System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogDebug(IPersistEntity^ entity, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogDebug(IPersistEntity^ entity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);
				void LogDebug(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ arg);

				void Log(LogLevel logLevel, System::Exception^ exception, IPersistEntity^ entity, System::String^ format, ...cli::array<System::Object^>^ args);

			public:

				FactoryBase(Xbim::Geometry::Services::ModelService^ modelService, T* nativeFactory);
				virtual property IXModelService^ ModelService {IXModelService^ get(); }
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get(); }
			};


		}
	}
}

