#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include "./Unmanaged/NSolidFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
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
			public ref class SolidFactory : XbimHandle<NSolidFactory>
			{
				LoggingService^ LoggerService;
				ILogger^ Logger;
				IModel^ _ifcModel;
				//The distance between two points at which they are determined to be equal points
				double _modelTolerance;
				GeomProcFactory^ _gpFactory;
			public:
				SolidFactory(LoggingService^ loggingService, IModel^ ifcModel) : XbimHandle(new NSolidFactory(loggingService))
				{
					LoggerService = loggingService;
					Logger = LoggerService->Logger;
					_modelTolerance = ifcModel->ModelFactors->Precision;
					_ifcModel = ifcModel;
					_gpFactory = gcnew GeomProcFactory();
				}
				//Builds all IfcSolidModels
				//throws XbimGeometryFactoryException if the solid cannot be built
				IXSolid^ Build(IIfcSolidModel^ ifcSolid);
				IXSolid^ Build(IIfcCsgPrimitive3D^ ifcCsgPrimitive);
				TopoDS_Solid BuildSolidModel(IIfcSolidModel^ ifcSolid);
#pragma region CSG solids
				TopoDS_Solid BuildCsgSolid(IIfcCsgSolid^ ifcCsgSolid);
				TopoDS_Solid BuildBooleanResult(IIfcBooleanResult^ ifcBooleanResult);
				TopoDS_Solid BuildCsgPrimitive3D(IIfcCsgPrimitive3D^ ifcCsgPrimitive3D);
				TopoDS_Solid BuildBlock(IIfcBlock^ ifcBlock);
				TopoDS_Solid BuildRectangularPyramid(IIfcRectangularPyramid^ ifcRectangularPyramid);
				TopoDS_Solid BuildRightCircularCone(IIfcRightCircularCone^ ifcRightCircularCone);
				TopoDS_Solid BuildRightCircularCylinder(IIfcRightCircularCylinder ^ (ifcRightCircularCylinder));
				TopoDS_Solid BuildSphere(IIfcSphere^ ifcSphere);
#pragma endregion
			};
		}
	}
}
