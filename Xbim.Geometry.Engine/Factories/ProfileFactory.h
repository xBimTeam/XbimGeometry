#include "../XbimHandle.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include "./Unmanaged/NProfileFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
#include "CurveFactory.h"
#include "WireFactory.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ProfileFactory : XbimHandle<NProfileFactory>
			{
			private:
				LoggingService^ LoggerService;
				ILogger^ Logger;
				IModel^ _ifcModel;
				//The distance between two points at which they are determined to be equal points
				double _modelTolerance;
				GeomProcFactory^ _gpFactory;
				CurveFactory^ _curveFactory;
				WireFactory^ _wireFactory;
			public:
				ProfileFactory(LoggingService^ loggingService, IModel^ ifcModel) : XbimHandle(new NProfileFactory(loggingService))
				{
					LoggerService = loggingService;
					Logger = LoggerService->Logger;
					_modelTolerance = ifcModel->ModelFactors->Precision;
					_ifcModel = ifcModel;
					_gpFactory = gcnew GeomProcFactory();
					_wireFactory = gcnew WireFactory(loggingService, ifcModel);
				}

				//Returns a compound where the CURVE profiles that have more than one wire, a wire for profiles that are defined as CURVES with one wire or a face for AREA types
				TopoDS_Shape Build(IIfcProfileDef^ profileDef);
				TopoDS_Shape Build(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile);
			};
		}
	}
}

