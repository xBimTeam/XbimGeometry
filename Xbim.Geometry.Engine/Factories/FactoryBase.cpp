#include "FactoryBase.h"
#include "../Exceptions//XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
using namespace Microsoft::Extensions::Logging;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			template<class T>
			FactoryBase<T>::FactoryBase(Xbim::Geometry::Services::ModelService^ modelService, T* nativeFactory) : XbimHandle(nativeFactory)
			{
				_modelService = modelService;
				NLoggingService* logService = new NLoggingService();
				logService->SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
				EXEC_NATIVE->SetLogger(logService);
			}

			template<class T>
			void FactoryBase<T>::LogWarning(System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Warning, nullptr, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogWarning(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Warning, exception, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogWarning(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Warning, nullptr, ifcEntity, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogWarning(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Warning, exception, nullptr, format, args);
			}

			template<class T>
			void FactoryBase<T>::LogInformation(System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Information, nullptr, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogInformation(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Information, exception, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogInformation(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Information, nullptr, ifcEntity, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogInformation(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Information, exception, nullptr, format, args);
			}

			template<class T>
			void FactoryBase<T>::LogError(System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Error, nullptr, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogError(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Error, exception, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogError(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Error, nullptr, ifcEntity, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogError(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Error, exception, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogDebug(System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Debug, nullptr, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogDebug(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Debug, exception, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogDebug(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Debug, nullptr, ifcEntity, format, args);
			}
			template<class T>
			void FactoryBase<T>::LogDebug(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				Log(LogLevel::Debug, exception, nullptr, format, args);
			}
			template<class T>
			void FactoryBase<T>::Log(LogLevel logLevel, System::Exception^ exception, IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
			{
				System::String^ msg = System::String::Format(format, arg);

				if (ifcEntity != nullptr)
				{
					System::String^ amendedMessage = "#{EntityId}={EntityType}: " + message;
					cli::array<System::Object^>^ amendedArgs = gcnew cli::array<System::Object^>(args->Count + 2);
					amendedArgs[0] = ifcEntity->EntityLabel;
					amendedArgs[1] = ifcEntity->GetType()->Name;
					for (size_t i = 0; i < args->Count; i++) amendedArgs[i + 2] = args[i];
					LoggerExtensions::Log(LoggingService->Logger, logLevel, 0, exception, amendedMessage, amendedArgs);
				}
				else
					LoggerExtensions::Log(LoggingService->Logger, logLevel, 0, exception, format, args);
			}

			template<class T>
			IXModelService^ FactoryBase<T>::ModelService::get()
			{
				return _modelService;
			};
			template<class T>
			IXLoggingService^ FactoryBase<T>::LoggingService::get()
			{
				return _modelService->LoggingService;
			};

			template<class T>
			void FactoryBase<T>::RaiseGeometryFactoryException(System::String^ message)
			{
				RaiseGeometryFactoryException(message, nullptr, nullptr);
			}
			template<class T>
			void FactoryBase<T>::RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity)
			{
				RaiseGeometryFactoryException(message, entity, nullptr);
			}
			template<class T>
			void FactoryBase<T>::RaiseGeometryFactoryException(System::String^ message, System::Exception^ innerException)
			{
				RaiseGeometryFactoryException(message, nullptr, exception);
			}
			template<class T>
			void FactoryBase<T>::RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity, System::Exception^)
			{
				XbimGeometryFactoryException^ geomExcept = gcnew XbimGeometryFactoryException(message, innerException);
				if (entity != nullptr)
				{
					geomExcept->Data->Add("IfcEntityId", entity->EntityLabel);
					geomExcept->Data->Add("IfcEntityType", entity->GetType()->Name);
					System::String^ formattedMessage = "#{EntityId}={EntityType}: " + message;
					LoggerExtensions::LogWarning(_modelService->LoggingService->Logger, geomExcept, formattedMessage, entity->EntityLabel, entity->GetType()->Name);
				}
				else
					LoggerExtensions::LogWarning(_modelService->LoggingService->Logger, geomExcept, message)
					throw geomExcept;
			}

		}
	}
}
