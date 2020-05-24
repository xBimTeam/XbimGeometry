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
				IXLoggingService^ LoggerService;			
				IXModelService^ ModelService;

				//The distance between two points at which they are determined to be equal points
				
				GeomProcFactory^ _gpFactory;
				virtual property double ModelTolerance  {double get() sealed { return ModelService->Precision; } };
			public:
				SolidFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NSolidFactory())
				{
					LoggerService = loggingService;					
					_gpFactory = gcnew GeomProcFactory(loggingService, modelService);
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				//Builds all IfcSolidModels
				//throws XbimGeometryFactoryException if the solid cannot be built
				
				virtual IXSolid^ Build(IIfcSolidModel^ ifcSolid);
				virtual IXSolid^ Build(IIfcCsgPrimitive3D^ ifcCsgPrimitive);
				virtual IXSolid^ Build(IIfcBooleanOperand^ boolOperand);
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
