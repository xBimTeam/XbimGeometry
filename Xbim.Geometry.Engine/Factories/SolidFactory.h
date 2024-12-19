#pragma once
#include "FactoryBase.h"
#include <TopoDS_Solid.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
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

				virtual property double ModelTolerance {double get() sealed { return ModelGeometryService->Precision; } };
			public:
				SolidFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NSolidFactory()) {}
				//Builds all IfcSolidModels
				//throws XbimGeometryFactoryException if the solid cannot be built

				bool TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape);

				IXShape^ Convert(System::String^ brepStr);

				virtual IXShape^ Build(IIfcSolidModel^ ifcSolid);
				virtual IXShape^ Build(IIfcFacetedBrep^ ifcBrep);
				virtual IXShape^ Build(IIfcFaceBasedSurfaceModel^ ifcSurfaceModel);
				virtual IXSolid^ Build(IIfcCsgPrimitive3D^ ifcCsgPrimitive);
				virtual IXSolid^ Build(IIfcHalfSpaceSolid^ ifcHalfSpaceSolid);
				virtual IXShape^ Build(IIfcShellBasedSurfaceModel^ ifcSurfaceModel);
				virtual IXShape^ Build(IIfcTessellatedItem^ ifcTessellatedItem);
				virtual IXShape^ Build(IIfcSectionedSpine^ ifcSectionedSpine);

				TopoDS_Shape BuildSolidModel(IIfcSolidModel^ ifcSolid);
				TopoDS_Shape BuildSurfaceCurveSweptAreaSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSurfaceCurveSweptAreaSolid);
				TopoDS_Solid BuildSweptDiskSolidPolygonal(IIfcSweptDiskSolidPolygonal^ ifcSweptDiskSolidPolygonal);

#pragma region CSG solids		

				TopoDS_Solid BuildCsgSolid(IIfcCsgSolid^ ifcCsgSolid);
				TopoDS_Solid BuildBooleanResult(IIfcBooleanResult^ ifcBooleanResult);
				TopoDS_Solid BuildCsgPrimitive3D(IIfcCsgPrimitive3D^ ifcCsgPrimitive3D);
				TopoDS_Solid BuildBlock(IIfcBlock^ ifcBlock);
				TopoDS_Solid BuildRectangularPyramid(IIfcRectangularPyramid^ ifcRectangularPyramid);
				TopoDS_Solid BuildRightCircularCone(IIfcRightCircularCone^ ifcRightCircularCone);
				TopoDS_Solid BuildRightCircularCylinder(IIfcRightCircularCylinder ^ (ifcRightCircularCylinder));
				TopoDS_Solid BuildSphere(IIfcSphere^ ifcSphere);

				TopoDS_Solid BuildHalfSpace(IIfcHalfSpaceSolid^ ifcHalfSpaceSolid);

#pragma endregion

#pragma region Swept solids
				TopoDS_Solid BuildSweptDiskSolid(IIfcSweptDiskSolid^ ifcSolid);
				TopoDS_Shape BuildExtrudedAreaSolidTapered(IIfcExtrudedAreaSolidTapered^ extrudedSolid);
				TopoDS_Shape BuildExtrudedAreaSolid(IIfcExtrudedAreaSolid^ extrudedSolid);
				TopoDS_Solid BuildSurfaceCurveSweptAreaSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSurfaceCurveSweptAreaSolid, IIfcProfileDef^ profileDef);
#pragma endregion
				
				TopoDS_Shape BuildAdvancedBrep(IIfcAdvancedBrep^ ifcAdvancedBrep);
				TopoDS_Shape BuildFacetedBrep(IIfcFacetedBrep^ facetedBrep);
				TopoDS_Shape BuildFaceBasedSurfaceModel(IIfcFaceBasedSurfaceModel^ faceBasedSurfaceModel);
				TopoDS_Shape BuildPolygonalFaceSet(IIfcPolygonalFaceSet^ ifcPolygonalFaceSet);

				TopoDS_Shape BuildExtrudedAreaSolid(IIfcExtrudedAreaSolid^ extrudedSolid, IIfcProfileDef^ profileDef);
				TopoDS_Shape BuildExtrudedAreaSolidTapered(IIfcExtrudedAreaSolidTapered^ extrudedSolid, IIfcProfileDef^ profileDef, IIfcProfileDef^ endProfileDef);
				
				TopoDS_Shape BuildSectionedSolidHorizontal(Xbim::Ifc4x3::GeometricModelResource::IfcSectionedSolidHorizontal^ sectionedSolidHorizontal);
				TopoDS_Shape BuildDirectrixDerivedReferenceSweptAreaSolid(Xbim::Ifc4x3::GeometricModelResource::IfcDirectrixDerivedReferenceSweptAreaSolid^ directrixDerivedReferenceSweptAreaSolid);


			private:
				void MoveSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Wire& section);
				void MoveSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Face& section);
				TopoDS_Face MovedSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Face& section);
				TopoDS_Wire MovedSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Wire& section);
			};
		}
	}
}
