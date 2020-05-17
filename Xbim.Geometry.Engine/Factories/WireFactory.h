#include "../XbimHandle.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include "./Unmanaged/NWireFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
#include "CurveFactory.h"
#include <BRep_Builder.hxx>
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
			public ref class WireFactory : XbimHandle<NWireFactory>
			{
			private:
				LoggingService^ LoggerService;
				ILogger^ Logger;
				IModel^ _ifcModel;
				//The distance between two points at which they are determined to be equal points
				double _modelTolerance;
				GeomProcFactory^ _gpFactory;
				CurveFactory^ _curveFactory;
				TopoDS_Wire Build2d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build3d(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				TopoDS_Wire Build2dCircle(IIfcCircle^ ifcCircle, Handle(Geom_Surface)& surface);
				
			public:
				WireFactory(LoggingService^ loggingService, IModel^ ifcModel) : XbimHandle(new NWireFactory(loggingService))
				{
					LoggerService = loggingService;
					Logger = LoggerService->Logger;
					_modelTolerance = ifcModel->ModelFactors->Precision;
					_gpFactory = gcnew GeomProcFactory();
					_curveFactory = gcnew CurveFactory(loggingService,ifcModel);
				}
				//Builds an IfcCurve as a TopoDS_Wire
				TopoDS_Wire Build(IIfcCurve^ ifcCurve, Handle(Geom_Surface)& surface);
				
			};

		}
	}
}

