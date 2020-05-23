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
			public ref class ProfileFactory : XbimHandle<NProfileFactory>, IXProfileService
			{
			private:
				IXLoggingService^ LoggerService;				
				IXModelService^ _ifcModel;				
				GeomProcFactory^ GPFactory;
				CurveFactory^ _curveFactory;
				WireFactory^ _wireFactory;
			public:
				ProfileFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NProfileFactory())
				{
					LoggerService = loggingService;									
					_ifcModel = modelService;
					GPFactory = gcnew GeomProcFactory(loggingService, modelService);
					_wireFactory = gcnew WireFactory(loggingService, modelService);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				virtual IXShape^ Build(IIfcProfileDef^ profileDef);

				//Returns a compound where the CURVE profiles that have more than one wire, a wire for profiles that are defined as CURVES with one wire or a face for AREA types
				TopoDS_Shape BuildShape(IIfcProfileDef^ profileDef);
				TopoDS_Shape BuildShape(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile);
			};
		}
	}
}

