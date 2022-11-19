#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include "./Unmanaged/NSolidFactory.h"
#include "../Services/LoggingService.h"
#include "GeometryFactory.h"
#include "CurveFactory.h"
#include "WireFactory.h"
#include "FaceFactory.h"
#include "ShellFactory.h"
#include "ShapeFactory.h"
#include "CompoundFactory.h"
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
			public ref class SolidFactory : IXSolidFactory, XbimHandle<NSolidFactory>
			{
					
				IXModelService^ _modelService;		
				virtual property double ModelTolerance  {double get() sealed { return ModelService->Precision; } };
			public:
				SolidFactory(ModelService^ modelService) : XbimHandle(new NSolidFactory())
				{
						
					_modelService = modelService;					
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				//Builds all IfcSolidModels
				//throws XbimGeometryFactoryException if the solid cannot be built
				
				bool TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape);

				virtual IXShape^ Build(IIfcSolidModel^ ifcSolid);
				virtual IXShape^ Build(IIfcFacetedBrep^ ifcBrep);
				virtual IXShape^ Build(IIfcFaceBasedSurfaceModel^ ifcSurfaceModel);
				virtual IXSolid^ Build(IIfcCsgPrimitive3D^ ifcCsgPrimitive);
				
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _modelService->LoggingService; }};

				TopoDS_Shape BuildSolidModel(IIfcSolidModel^ ifcSolid);

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

				TopoDS_Shape BuildFacetedBrep(IIfcFacetedBrep^ facetedBrep);

				TopoDS_Shape BuildFaceBasedSurfaceModel(IIfcFaceBasedSurfaceModel^ faceBasedSurfaceModel);

				
			};
		}
	}
}
