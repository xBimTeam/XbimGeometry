#pragma once
#include "FactoryBase.h"
#include "./Unmanaged/NFaceFactory.h"

#include <TopTools_SequenceOfShape.hxx>
#include <TopTools_DataMapOfIntegerShape.hxx>

using namespace Xbim::Geometry::Services;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class FaceFactory : FactoryBase<NFaceFactory>, IXFaceFactory
			{

			internal:
				//TopoDS_Face BuildProfileDef(IIfcProfileDef^ profileDef);
				gp_Vec Normal(const TopoDS_Face& face);
			public:

				FaceFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NFaceFactory()) {}

#pragma region Interface implementations

				virtual IXFace^ BuildFace(IXSurface^ surface, array<IXWire^>^ wires);
				
#pragma endregion

#pragma region Native methods


				TopoDS_Face BuildPlanarFace(IXPlane^ planeDef);
				TopoDS_Face BuildAdvancedFace(IIfcAdvancedFace^ advancedFace, TopTools_DataMapOfIntegerShape& edgeCurves, TopTools_DataMapOfIntegerShape& vertices);
				TopoDS_Face BuildFace(const Handle(Geom_Surface)& surface);
				//TopoDS_Face BuildFace(const Handle(Geom_Surface)& surface, const TopoDS_Wire& outerLoop, const TopTools_SequenceOfShape& innerLoops);

#pragma endregion
			};
		}
	}
}

