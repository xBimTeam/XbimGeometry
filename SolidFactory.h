#pragma once
#include "../XbimHandle.h"
#include "../Services/LoggingService.h"
#include "Unmanaged/NSolidFactory.h"
#include "../BRep/XbimSolid.h"
#include <TopoDS_Solid.hxx>
#include "CurveFactory.h"

using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class SolidFactory : XbimHandle<NSolidFactory>
			{
			private:
				LoggingService^ LoggerService;
				ILogger^ Logger;
				IModel^ ifcModel;
				//The distance between two points at which they are determined to be equal points
				double _modelTolerance;
				GeomProcFactory^ _gpFactory;
			public:
				SolidFactory(LoggingService^ loggingService, IModel^ ifcModel) : XbimHandle(new NSolidFactory(loggingService))
				{
					LoggerService = loggingService;
					Logger = LoggerService->Logger;
					_modelTolerance = ifcModel->ModelFactors->Precision;
					//_curveFactory = gcnew CurveFactory(loggingService, ifcModel);
					_gpFactory = gcnew GeomProcFactory();
				};
				//Builds all IfcSolidModels
				//throws XbimGeometryFactoryException if the solid cannot be built
				IXSolid^ Build(IIfcSolidModel^ ifcSolid);
				const TopoDS_Solid& BuildSolidModel(IIfcSolidModel^ ifcSolid);
#pragma region CSG solids
				const TopoDS_Solid& BuildCsgSolid(IIfcCsgSolid^ ifcCsgSolid);
				const TopoDS_Solid& BuildBooleanResult(IIfcBooleanResult^ ifcBooleanResult);
				const TopoDS_Solid& BuildCsgPrimitive3D(IIfcCsgPrimitive3D^ ifcCsgPrimitive3D);
				const TopoDS_Solid& BuildBlock(IIfcBlock^ ifcBlock);
				const TopoDS_Solid& BuildRectangularPyramid(IIfcRectangularPyramid^ ifcRectangularPyramid);
				const TopoDS_Solid& BuildRightCircularCone(IIfcRightCircularCone^ ifcRightCircularCone);
				const TopoDS_Solid& BuildRightCircularCylinder(IIfcRightCircularCylinder^ (ifcRightCircularCylinder));
				const TopoDS_Solid& BuildSphere(IIfcSphere^ ifcSphere);
#pragma endregion
			};
		}
	}
}

