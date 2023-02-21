#pragma once
#include "../XbimHandle.h"
#include "../Services/ModelGeometryService.h"

using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Abstractions;

#define EXEC_NATIVE Ptr()
#define VERTEX_FACTORY _modelService->GetVertexFactory()
#define SURFACE_FACTORY _modelService->GetSurfaceFactory()
#define CURVE_FACTORY _modelService->GetCurveFactory()
#define GEOMETRY_FACTORY _modelService->GetGeometryFactory()
#define EDGE_FACTORY _modelService->GetEdgeFactory()
#define WIRE_FACTORY _modelService->GetWireFactory()
#define FACE_FACTORY _modelService->GetFaceFactory()
#define SHELL_FACTORY _modelService->GetShellFactory()
#define PROFILE_FACTORY _modelService->GetProfileFactory()
#define SHAPE_FACTORY _modelService->GetShapeFactory()
#define SOLID_FACTORY _modelService->GetSolidFactory()
#define BIM_WORKAROUNDS _modelService->GetBimAuthoringToolWorkArounds()
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			ref class ModelGeometryService;
		}
	}
}
using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			template <class T>
			public ref class FactoryBase abstract : public XbimHandle<T>, IXModelScoped
			{
			protected:
				Xbim::Geometry::Services::ModelGeometryService^ _modelService;
				XbimGeometryFactoryException^ RaiseGeometryFactoryException(System::String^ message) { return RaiseGeometryFactoryException(message, nullptr, nullptr); };
				XbimGeometryFactoryException^ RaiseGeometryFactoryException(System::String^ message, System::Exception^ innerException) { return RaiseGeometryFactoryException(message, nullptr, innerException); }
				XbimGeometryFactoryException^ RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity) { return RaiseGeometryFactoryException(message, entity, nullptr); }
				XbimGeometryFactoryException^ RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity, System::Exception^ innerException)
				{
					XbimGeometryFactoryException^ geomExcept = gcnew XbimGeometryFactoryException(message, innerException);
					if (entity != nullptr)
					{
						geomExcept->Data->Add("IfcEntityId", entity->EntityLabel);
						geomExcept->Data->Add("IfcEntityType", entity->GetType()->Name);
						/* We're using the XbimGeometryFactoryException.Data Dictionary as a convenient contained of the State for a Logger scope, so
						we can log the inner exception rather than the generic wrapper exception. Provides the same context and more detail on the original fault. */
						System::IDisposable^ scope = nullptr;
						try
						{
							scope = _modelService->LoggingService->Logger->BeginScope(geomExcept->Data);
							LoggerExtensions::LogWarning(_modelService->LoggingService->Logger, innerException, message);
						}
						finally
						{
							if (scope != nullptr)
								delete scope;
						}
					}
					else
						LoggerExtensions::LogWarning(_modelService->LoggingService->Logger, innerException, message);
					return geomExcept;
				}
				void LogError(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Error, nullptr, nullptr, format, args); };
				void LogError(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, nullptr, ifcEntity, format, args); };
				void LogError(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Error, exception, ifcEntity, format, args); };
				void LogError(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Error, exception, nullptr, format, args); };

				void LogWarning(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, nullptr, nullptr, format, args); };
				void LogWarning(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, nullptr, ifcEntity, format, args); };
				void LogWarning(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, exception, ifcEntity, format, args); };
				void LogWarning(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, exception, nullptr, format, args); };

				void LogInformation(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, nullptr, nullptr, format, args); };
				void LogInformation(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, nullptr, ifcEntity, format, args); };
				void LogInformation(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, exception, ifcEntity, format, args); };
				void LogInformation(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, exception, nullptr, format, args); };

				void LogDebug(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, nullptr, nullptr, format, args); };
				void LogDebug(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, nullptr, ifcEntity, format, args); };
				void LogDebug(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, exception, ifcEntity, format, args); };
				void LogDebug(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, exception, nullptr, format, args); };

				void Log(LogLevel logLevel, System::Exception^ exception, IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
				{
					if (!LoggingService->Logger->IsEnabled(logLevel))
						return;
					System::String^ message = System::String::Format(format, args);

					if (ifcEntity != nullptr)
					{
						System::String^ amendedMessage = "#{EntityId}={EntityType}: " + message;
						cli::array<System::Object^>^ amendedArgs = gcnew cli::array<System::Object^>(args->Length + 2);
						amendedArgs[0] = ifcEntity->EntityLabel;
						amendedArgs[1] = ifcEntity->GetType()->Name;
						for (int i = 0; i < args->Length; i++) amendedArgs[i + 2] = args[i];
						LoggerExtensions::Log(LoggingService->Logger, logLevel, 0, exception, amendedMessage, amendedArgs);
					}
					else
						LoggerExtensions::Log(LoggingService->Logger, logLevel, 0, exception, format, args);
				}
				ILogger^ Logger() { return _modelService->LoggingService->Logger; };

				ModelGeometryService^ GetModelModelGeometryService() {
					return _modelService;
				};
			public:


				FactoryBase<T>(ModelGeometryService^ modelService, T* nativeFactory) : XbimHandle<T>(nativeFactory)
				{
					_modelService = modelService;
					OccHandle().SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
				}

				virtual property IXModelGeometryService^ ModelGeometryService {IXModelGeometryService^ get() { return _modelService; }; }
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _modelService->LoggingService; }; }
			};


		}
	}
}

