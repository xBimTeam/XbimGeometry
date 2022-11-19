#include "FactoryBase.h"
#include "../Exceptions//XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
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
			void FactoryBase<T>::RaiseGeometryFactoryException(System::String^ message, IPersistEntity^ entity, System::Exception^ )
			{				
				XbimGeometryFactoryException^ geomExcept = gcnew XbimGeometryFactoryException(message, innerException);
				if (entity != nullptr)
				{				
					geomExcept->Data->Add("IfcEntityId", entity->EntityLabel);
					geomExcept->Data->Add("IfcEntityType", entity->GetType()->Name);
					System::String^ formattedMessage = "#{EntityId}={EntityType}: " + message;
					Microsoft::Extensions::Logging::LoggerExtensions::LogWarning(_modelService->LoggingService->Logger, geomExcept, formattedMessage, entity->EntityLabel, entity->GetType()->Name);
				}
				else
					Microsoft::Extensions::Logging::LoggerExtensions::LogWarning(_modelService->LoggingService->Logger, geomExcept, message)
				throw geomExcept;
			}

		}
	}
}
