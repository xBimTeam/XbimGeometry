#pragma once
#include "FactoryBase.h"
#include "./Unmanaged/NFaceFactory.h"


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
			
				FaceFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NFaceFactory()) {}
			
				TopoDS_Face BuildPlanarFace(IXPlane^ planeDef);
				
				virtual IXFace^ BuildFace(IXSurface^ surface, array<IXWire^>^ wires);
			
			};
		}
	}
}

