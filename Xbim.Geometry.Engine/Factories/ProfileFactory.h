#pragma once

#include "./Unmanaged/NProfileFactory.h"
#include "FactoryBase.h"
#include "GeometryFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class ProfileFactory : public FactoryBase<NProfileFactory>, IXProfileFactory
			{	
			private:
				GeometryFactory^ _geometryFactory;
			public:
				ProfileFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NProfileFactory()) 
				{
					_geometryFactory = gcnew GeometryFactory(modelService);
				};

				virtual IXFace^ BuildFace(IIfcProfileDef^ profileDef);
				virtual IXWire^ BuildWire(IIfcProfileDef^ profileDef);
				TopoDS_Wire BuildProfileWire(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile);
				TopoDS_Wire BuildProfileWire(IIfcCircleProfileDef^ arbitraryClosedProfile);
				TopoDS_Wire BuildProfileWire(IIfcRectangleProfileDef^ rectangleProfile);
				virtual IXEdge^ BuildEdge(IIfcProfileDef^ profileDef);
			internal:
				
				TopoDS_Face BuildProfileFace(const TopoDS_Wire& wire);
				TopoDS_Face BuildProfileFace(IIfcProfileDef^ ifcProfileDef);
				TopoDS_Face BuildProfileFace(IIfcArbitraryProfileDefWithVoids^ arbitraryClosedProfile);
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
				TopoDS_Face BuildProfileFace(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef);
				TopoDS_Face BuildProfileFace(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef);
				//constructs a reProfilectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
				TopoDS_Face BuildProfileFace(double x, double y, double tolerance, bool centre);

				TopoDS_Edge BuildProfileEdge(IIfcProfileDef^ ifcProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcDerivedProfileDef^ ifcDerivedProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcParameterizedProfileDef^ ifcParameterizedProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcCircleProfileDef^ ifcCircleProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcRectangleProfileDef^ ifcRectangleProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcRoundedRectangleProfileDef^ ifcRoundedRectangleProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcLShapeProfileDef^ ifcLShapeProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcUShapeProfileDef^ ifcUShapeProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcEllipseProfileDef^ ifcEllipseProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcIShapeProfileDef^ ifcIShapeProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcZShapeProfileDef^ ifcZShapeProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcCShapeProfileDef^ ifcCShapeProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcTShapeProfileDef^ ifcTShapeProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef);
			protected:
				
			};
		}
	}
}

