#pragma once

#include "./Unmanaged/NProfileFactory.h"
#include "FactoryBase.h"


namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ProfileFactory : FactoryBase<NProfileFactory>, IXProfileFactory
			{	
			public:
				ProfileFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NProfileFactory()) {};

				virtual IXFace^ BuildFace(IIfcProfileDef^ profileDef);
				virtual IXWire^ BuildWire(IIfcProfileDef^ profileDef);
				virtual IXEdge^ BuildEdge(IIfcProfileDef^ profileDef);
			internal:
				
				TopoDS_Face BuildProfileFace(const TopoDS_Wire& wire);
				TopoDS_Face BuildProfileFace(IIfcProfileDef^ ifcProfileDef);
				TopoDS_Face BuildProfileFace(IIfcDerivedProfileDef^ ifcDerivedProfileDef);
				TopoDS_Face BuildProfileFace(IIfcParameterizedProfileDef^ ifcParameterizedProfileDef);
				TopoDS_Face BuildProfileFace(IIfcCircleProfileDef^ ifcCircleProfileDef);
				TopoDS_Face BuildProfileFace(IIfcRectangleProfileDef^ ifcRectangleProfileDef);
				TopoDS_Face BuildProfileFace(IIfcRoundedRectangleProfileDef^ ifcRoundedRectangleProfileDef);
				TopoDS_Face BuildProfileFace(IIfcLShapeProfileDef^ ifcLShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcUShapeProfileDef^ ifcUShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcEllipseProfileDef^ ifcEllipseProfileDef);
				TopoDS_Face BuildProfileFace(IIfcIShapeProfileDef^ ifcIShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcZShapeProfileDef^ ifcZShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcCShapeProfileDef^ ifcCShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcTShapeProfileDef^ ifcTShapeProfileDef);						
				TopoDS_Face BuildProfileFace(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef);
				TopoDS_Face BuildProfileFace(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef);
				//constructs a reProfilectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
				TopoDS_Face BuildProfileFace(double x, double y, double tolerance, bool centre);
			protected:
				
			};
		}
	}
}

