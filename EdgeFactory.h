#pragma once
#include "../XbimHandle.h"
#include "../Services/LoggingService.h"
#include "Unmanaged/NEdgeFactory.h"
#include "../BRep/XbimEdge.h"

#include "CurveFactory.h"
#include <TopoDS_Edge.hxx>
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class EdgeFactory : XbimHandle<NEdgeFactory>
			{
			private:
				LoggingService^ LoggerService;
				ILogger^ Logger;
				IModel^ ifcModel;
				//The distance between two points at which they are determined to be equal points
				double _modelTolerance;
				CurveFactory^ _curveFactory;
			public:
				EdgeFactory(LoggingService^ loggingService, IModel^ ifcModel) : XbimHandle(new NEdgeFactory(loggingService))
				{
					LoggerService = loggingService;
					Logger = LoggerService->Logger;
					_modelTolerance = ifcModel->ModelFactors->Precision;
					_curveFactory = gcnew CurveFactory(loggingService, ifcModel);
				}
				XbimEdge^ BuildEdge();

				const TopoDS_Edge& BuildTopoEdge(IIfcCurve^ curve);
			};
		}
	}
}

