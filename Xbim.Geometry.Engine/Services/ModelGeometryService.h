#pragma once
#pragma warning( disable : 4691 )
#include "MeshFactors.h"
#include "LoggingService.h"
#include "../XbimGeometryCreator.h"
#include "../Exceptions/XbimGeometryServiceException.h"
#include "../Exceptions/XbimGeometryServiceException.h"
using namespace Xbim::Geometry::Exceptions;

#define ActiveModelGeometryService(ifcEntity) static_cast<ModelGeometryService^>(static_cast<IPersistEntity^>(ifcEntity)->Model->Tag)


using namespace Xbim::Common;
using namespace Xbim::Geometry;
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Ifc4::Interfaces;
using namespace System::Linq;
using namespace System::Collections::Generic;
using namespace Microsoft::Extensions::Logging;
using namespace Microsoft::Extensions::Logging::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		//ref class GeometryCreator;
		namespace Factories
		{
			//forward declare all factories
			ref class VertexFactory;
			ref class GeometryFactory;
			ref class CurveFactory;
			ref class SurfaceFactory;
			ref class EdgeFactory;
			ref class WireFactory;
			ref class FaceFactory;
			ref class ShellFactory;
			ref class SolidFactory;
			ref class CompoundFactory;
			ref class BooleanFactory;
			ref class ShapeFactory;
			ref class ProfileFactory;
			ref class BIMAuthoringToolWorkArounds;
			ref class MaterialFactory;
			ref class ProjectionFactory;
		}
	}
}

namespace Xbim
{
	namespace Geometry
	{

		namespace Services
		{

			public ref class ModelGeometryService : IXModelGeometryService
			{
			protected:
				LoggingService^ _loggingService;
			private:
				IModel^ model;
				double minimumGap;
				double precisionSquared;
				double minAreaM2;

				double _timeout;
				bool _upgradeFaceSets = true;

				Xbim::Geometry::Factories::VertexFactory^ _vertexFactory;
				Xbim::Geometry::Factories::GeometryFactory^ _geometryFactory;
				Xbim::Geometry::Factories::CurveFactory^ _curveFactory;
				Xbim::Geometry::Factories::SurfaceFactory^ _surfaceFactory;
				Xbim::Geometry::Factories::EdgeFactory^ _edgeFactory;
				Xbim::Geometry::Factories::WireFactory^ _wireFactory;
				Xbim::Geometry::Factories::FaceFactory^ _faceFactory;
				Xbim::Geometry::Factories::ShellFactory^ _shellFactory;
				Xbim::Geometry::Factories::SolidFactory^ _solidFactory;
				Xbim::Geometry::Factories::CompoundFactory^ _compoundFactory;
				Xbim::Geometry::Factories::BooleanFactory^ _booleanFactory;
				Xbim::Geometry::Factories::ShapeFactory^ _shapeFactory;
				Xbim::Geometry::Factories::ProfileFactory^ _profileFactory;
				Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds^ _bimAuthoringToolWorkArounds;
				Xbim::Geometry::Factories::MaterialFactory^ _materialFactory;
				Xbim::Geometry::Factories::ProjectionFactory^ _projectionFactory;
			internal:
				//Factories
				Xbim::Geometry::Factories::VertexFactory^ GetVertexFactory();
				Xbim::Geometry::Factories::GeometryFactory^ GetGeometryFactory();
				Xbim::Geometry::Factories::CurveFactory^ GetCurveFactory();
				Xbim::Geometry::Factories::SurfaceFactory^ GetSurfaceFactory();
				Xbim::Geometry::Factories::EdgeFactory^ GetEdgeFactory();
				Xbim::Geometry::Factories::WireFactory^ GetWireFactory();
				Xbim::Geometry::Factories::FaceFactory^ GetFaceFactory();
				Xbim::Geometry::Factories::ShellFactory^ GetShellFactory();
				Xbim::Geometry::Factories::SolidFactory^ GetSolidFactory();
				Xbim::Geometry::Factories::CompoundFactory^ GetCompoundFactory();
				Xbim::Geometry::Factories::BooleanFactory^ GetBooleanFactory();
				Xbim::Geometry::Factories::ShapeFactory^ GetShapeFactory();
				Xbim::Geometry::Factories::ProfileFactory^ GetProfileFactory();
				Xbim::Geometry::Factories::BIMAuthoringToolWorkArounds^ GetBimAuthoringToolWorkArounds();
				Xbim::Geometry::Factories::MaterialFactory^ GetMaterialFactory();
				Xbim::Geometry::Factories::ProjectionFactory^ GetProjectionFactory();
			public:

				ModelGeometryService(IModel^ model, ILoggerFactory^ loggerFactory);

				virtual property bool UpgradeFaceSets {bool get() { return _upgradeFaceSets; } void set(bool upgrade) { _upgradeFaceSets = upgrade; }};
				virtual property double Precision {double get() { return model->ModelFactors->Precision; }};
				virtual property double PrecisionSquared {double get() { return precisionSquared; }};
				virtual property double OneFoot {double get() { return model->ModelFactors->OneFoot; }};
				virtual property double OneMeter {double get() { return model->ModelFactors->OneMeter; }};
				virtual property double OneMillimeter {double get() { return model->ModelFactors->OneMilliMeter; }};
				virtual property double MinAreaM2 {double get() { return minAreaM2; }};
				virtual property double MinimumGap {double get() { return minimumGap; } void set(double distance) { minimumGap = distance; }};
				virtual property double Timeout {double get() { return _timeout; } void set(double timeout) { _timeout = timeout; }};
				virtual property double RadianFactor {double get() {
					double rad = model->ModelFactors->AngleToRadiansConversionFactor;
					return rad;

				}};

				virtual property IXMeshFactors^ MeshFactors {IXMeshFactors^ get() { return gcnew Xbim::Geometry::Services::MeshFactors(model->ModelFactors->OneMeter, model->ModelFactors->Precision); }; }
				virtual property IModel^ Model {IModel^ get() { return model; };  }
				virtual void SetModel(IModel^ model);
				virtual ISet<IIfcGeometricRepresentationContext^>^ GetTypical3dContexts();

				virtual IXLocation^ Create(IIfcObjectPlacement^ placement);
				virtual IXLocation^ CreateMappingTransform(IIfcMappedItem^ mappedItem);
				System::String^ GetBrep(const TopoDS_Shape& shape);

				//Factories
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get(); }
				virtual property IXVertexFactory^ VertexFactory {IXVertexFactory^ get(); }
				virtual property IXGeometryFactory^ GeometryFactory {IXGeometryFactory^ get(); }
				virtual property IXCurveFactory^ CurveFactory {IXCurveFactory^ get(); }
				virtual property IXSurfaceFactory^ SurfaceFactory {IXSurfaceFactory^ get(); }
				virtual property IXEdgeFactory^ EdgeFactory {IXEdgeFactory^ get(); }
				virtual property IXWireFactory^ WireFactory {IXWireFactory^ get(); }
				virtual property IXFaceFactory^ FaceFactory {IXFaceFactory^ get(); }
				virtual property IXShellFactory^ ShellFactory {IXShellFactory^ get(); }
				virtual property IXSolidFactory^ SolidFactory {IXSolidFactory^ get(); }
				virtual property IXCompoundFactory^ CompoundFactory {IXCompoundFactory^ get(); }
				virtual property IXBooleanFactory^ BooleanFactory {IXBooleanFactory^ get(); }
				virtual property IXShapeFactory^ ShapeFactory {IXShapeFactory^ get(); }
				virtual property IXProfileFactory^ ProfileFactory {IXProfileFactory^ get(); }
				virtual property IXMaterialFactory^ MaterialFactory {IXMaterialFactory^ get(); }
				virtual property IXProjectionFactory^ ProjectionFactory {IXProjectionFactory^ get(); }


#pragma region Logging and Exceptions
				XbimGeometryServiceException^ RaiseGeometryServiceException(System::String^ message) { return RaiseGeometryServiceException(message, nullptr, nullptr); };
				XbimGeometryServiceException^ RaiseGeometryServiceException(System::String^ message, System::Exception^ innerException) { return RaiseGeometryServiceException(message, nullptr, innerException); }
				XbimGeometryServiceException^ RaiseGeometryServiceException(System::String^ message, IPersistEntity^ entity) { return RaiseGeometryServiceException(message, entity, nullptr); }
				XbimGeometryServiceException^ RaiseGeometryServiceException(System::String^ message, IPersistEntity^ entity, System::Exception^ innerException)
				{
					XbimGeometryServiceException^ geomExcept = gcnew XbimGeometryServiceException(message, innerException);
					if (entity != nullptr)
					{
						geomExcept->Data->Add("IfcEntityId", entity->EntityLabel);
						geomExcept->Data->Add("IfcEntityType", entity->GetType()->Name);
						System::String^ formattedMessage = "#{EntityId}={EntityType}: " + message;
						LoggerExtensions::LogWarning(LoggingService->Logger, geomExcept, formattedMessage, entity->EntityLabel, entity->GetType()->Name);
					}
					else
						LoggerExtensions::LogWarning(LoggingService->Logger, geomExcept, message);
					return geomExcept;
				}
				virtual void LogError(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Error, nullptr, nullptr, format, args); };
				virtual void LogError(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, nullptr, ifcEntity, format, args); };
				virtual void LogError(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Error, exception, ifcEntity, format, args); };
				virtual void LogError(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Error, exception, nullptr, format, args); };
			
				virtual void LogWarning(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, nullptr, nullptr, format, args); };
				virtual void LogWarning(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, nullptr, ifcEntity, format, args); };
				virtual void LogWarning(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, exception, ifcEntity, format, args); };
				virtual void LogWarning(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Warning, exception, nullptr, format, args); };
			
				virtual void LogInformation(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, nullptr, nullptr, format, args); };
				virtual void LogInformation(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, nullptr, ifcEntity, format, args); };
				virtual void LogInformation(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, exception, ifcEntity, format, args); };
				virtual void LogInformation(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Information, exception, nullptr, format, args); };
			
				virtual void LogDebug(System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, nullptr, nullptr, format, args); };
				virtual void LogDebug(IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, nullptr, ifcEntity, format, args); };
				virtual void LogDebug(IPersistEntity^ ifcEntity, System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, exception, ifcEntity, format, args); };
				virtual void LogDebug(System::Exception^ exception, System::String^ format, ...cli::array<System::Object^>^ args) { Log(LogLevel::Debug, exception, nullptr, format, args); };

				void Log(LogLevel logLevel, System::Exception^ exception, IPersistEntity^ ifcEntity, System::String^ format, ...cli::array<System::Object^>^ args)
				{
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
				ILogger^ Logger() { return LoggingService->Logger; };
#pragma endregion

			};
		}
	}
}
