#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include "./Unmanaged/NSolidFactory.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
#include "CurveFactory.h"
#include "WireFactory.h"
#include "FaceFactory.h"
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
				FaceFactory^ _faceFactory;
				CurveFactory^ _curveFactory;
				WireFactory^ _wireFactory;
				//The distance between two points at which they are determined to be equal points
				
				GeomProcFactory^ _gpFactory;
				virtual property double ModelTolerance  {double get() sealed { return ModelService->Precision; } };
			public:
				SolidFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NSolidFactory())
				{
					LoggerService = loggingService;					
					_gpFactory = gcnew GeomProcFactory(loggingService, modelService);
					_curveFactory = gcnew CurveFactory(loggingService, modelService);
					_wireFactory = gcnew WireFactory(loggingService, modelService);
					_faceFactory = gcnew FaceFactory(loggingService, modelService);
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
#pragma region Swept solids
				TopoDS_Solid BuildSweptDiskSolid(IIfcSweptDiskSolid^ ifcSolid);
				TopoDS_Solid BuildExtrudedAreaSolid(IIfcExtrudedAreaSolid^ extrudedSolid);

#pragma endregion
			};
		}
	}
}
