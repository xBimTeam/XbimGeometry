#pragma once
#pragma warning( disable : 4691 )
#include "MeshFactors.h"
#include "LoggingService.h"
#include "../Factories/WireFactory.h"
#include "../Factories/CurveFactory.h"

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
				
				IXLoggingService^ _loggingService;
				IXWireFactory^ _wireFactory;
				IXCurveFactory^ _curveFactory;
				
			public:
				
				ModelService(IModel^ model, ILogger^ logger) ;
				
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
				virtual property IXWireFactory^ WireFactory {IXWireFactory^ get(); }
				virtual property IXCurveFactory^ CurveFactory {IXCurveFactory^ get(); }
			};
		}
	}
}
