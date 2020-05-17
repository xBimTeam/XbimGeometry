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
				IXLoggingService^ LoggerService;
				
				IXModelService^ _ifcModel;				
				GeomProcFactory^ _gpFactory;
				CurveFactory^ _curveFactory;
				WireFactory^ _wireFactory;
			public:
				ProfileFactory(IXLoggingService^ loggingService, IXModelService^ ifcModel) : XbimHandle(new NProfileFactory())
				{
					LoggerService = loggingService;									
					_ifcModel = ifcModel;
					_gpFactory = gcnew GeomProcFactory();
					_wireFactory = gcnew WireFactory(loggingService, ifcModel);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}

				//Returns a compound where the CURVE profiles that have more than one wire, a wire for profiles that are defined as CURVES with one wire or a face for AREA types
				TopoDS_Shape Build(IIfcProfileDef^ profileDef);
				TopoDS_Shape Build(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile);
			};
		}
	}
}

