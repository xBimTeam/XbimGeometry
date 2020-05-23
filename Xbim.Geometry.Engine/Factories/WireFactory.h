#include "../XbimHandle.h"
#include "../BRep/XbimWire.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include "../Factories/GeomProcFactory.h"
#include "./Unmanaged/NWireFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
#include "CurveFactory.h"
#include <BRep_Builder.hxx>
#include <Geom_Surface.hxx>

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
			public ref class WireFactory : XbimHandle<NWireFactory>, IXWireService
			{
			private:
				IXLoggingService^ LoggerService;
				
				IXModelService^ ModelService;
				//The distance between two points at which they are determined to be equal points
				
				GeomProcFactory^ GPFactory;
				CurveFactory^ _curveFactory;
				
				TopoDS_Wire Build3d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dCircle(IIfcCircle^ ifcCircle, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dTrimmedCurve(IIfcTrimmedCurve^ ifcTrimmedCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dPolyline(IIfcPolyline^ ifcPolyline, Handle(Geom_Surface)& surface);
				
			public:
				WireFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NWireFactory())
				{
					LoggerService = loggingService;		
					ModelService = modelService;
					GPFactory = gcnew GeomProcFactory(loggingService, modelService);
					_curveFactory = gcnew CurveFactory(loggingService, modelService);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				virtual IXWire^ Build(IIfcCurve^ ifcCurve);
				//Builds an IfcCurve as a TopoDS_Wire
				TopoDS_Wire BuildWire(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				
			};

		}
	}
}

