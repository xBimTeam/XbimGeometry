#pragma once
#pragma warning( disable : 4691 )
#include "MeshFactors.h"
#include "LoggingService.h"

#define ActiveModelService(ifcEntity) static_cast<ModelService^>(static_cast<IPersistEntity^>(ifcEntity)->Model->Tag)


using namespace Xbim::Common;
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
		namespace Factories
		{
			//forward declare all factories
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
		}
	}
}

namespace Xbim
{
	namespace Geometry
	{
		
		namespace Services
		{

			public ref class ModelService : IXModelService
			{
			private:
				IModel^ model;
				double minimumGap;
				double precisionSquared;
				double minAreaM2;

				double _timeout;
				bool _upgradeFaceSets = true;

				LoggingService^ _loggingService;
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
			internal:
				//Factories

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
			public:

				ModelService(IModel^ model, ILogger^ logger);

				virtual property bool UpgradeFaceSets {bool get() { return _upgradeFaceSets; } void set(bool upgrade) { _upgradeFaceSets = upgrade; }};
				virtual property double Precision {double get() { return model->ModelFactors->Precision; }};
				virtual property double PrecisionSquared {double get() { return precisionSquared; }};
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

				//Factories
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get(); }
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
			};
		}
	}
}
