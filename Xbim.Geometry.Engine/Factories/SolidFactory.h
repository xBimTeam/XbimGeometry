#pragma once
#include "FactoryBase.h"
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
			public ref class SolidFactory : IXSolidFactory, FactoryBase<NSolidFactory>
			{

				virtual property double ModelTolerance {double get() sealed { return ModelService->Precision; } };
			public:
				SolidFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NSolidFactory()) {}
				//Builds all IfcSolidModels
				//throws XbimGeometryFactoryException if the solid cannot be built

				bool TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape);

				virtual IXShape^ Build(IIfcSolidModel^ ifcSolid);
				virtual IXShape^ Build(IIfcFacetedBrep^ ifcBrep);
				virtual IXShape^ Build(IIfcFaceBasedSurfaceModel^ ifcSurfaceModel);
				virtual IXSolid^ Build(IIfcCsgPrimitive3D^ ifcCsgPrimitive);



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
