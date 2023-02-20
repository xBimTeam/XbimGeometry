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
				
			public:
				ProfileFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NProfileFactory()) 
				{				
				};

				virtual IXFace^ BuildFace(IIfcProfileDef^ profileDef);
				virtual IXWire^ BuildWire(IIfcProfileDef^ profileDef);
				TopoDS_Wire BuildProfileWire(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile);
				TopoDS_Wire BuildProfileWire(IIfcArbitraryOpenProfileDef^ arbitraryOpenProfile);
				TopoDS_Wire BuildProfileWire(IIfcCircleProfileDef^ arbitraryClosedProfile);
				TopoDS_Wire BuildProfileWire(IIfcRectangleProfileDef^ rectangleProfile);
				TopoDS_Wire BuildProfileWire(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef);
				virtual IXEdge^ BuildEdge(IIfcProfileDef^ profileDef);
			internal:
				
				
				TopoDS_Face BuildProfileFace(IIfcProfileDef^ ifcProfileDef);
				TopoDS_Face BuildProfileFace(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef);
				TopoDS_Face BuildProfileFace(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef);
				TopoDS_Face BuildProfileFace(IIfcArbitraryProfileDefWithVoids^ arbitraryClosedProfile);
				TopoDS_Face BuildProfileFace(IIfcAsymmetricIShapeProfileDef^ asymmetricIShapeProfile);
				TopoDS_Face BuildProfileFace(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef);
				TopoDS_Face BuildProfileFace(IIfcCircleHollowProfileDef^ ifcCircleHollowProfileDef);
				TopoDS_Face BuildProfileFace(IIfcCircleProfileDef^ circleProfile);
				TopoDS_Face BuildProfileFace(IIfcCompositeProfileDef^ ccmpositeProfile);
				TopoDS_Face BuildProfileFace(IIfcCShapeProfileDef^ ifcCShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcDerivedProfileDef^ ifcDerivedProfileDef);
				TopoDS_Face BuildProfileFace(IIfcEllipseProfileDef^ ellipseProfile);
				TopoDS_Face BuildProfileFace(IIfcIShapeProfileDef^ ifcIShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcLShapeProfileDef^ ifcLShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcMirroredProfileDef^ mirroredProfile);
				TopoDS_Face BuildProfileFace(IIfcRectangleHollowProfileDef^ rectangleHollowProfile);
				TopoDS_Face BuildProfileFace(IIfcRectangleProfileDef^ ifcRectangleProfileDef);	
				TopoDS_Face BuildProfileFace(IIfcRoundedRectangleProfileDef^ roundedRectangleProfile);
				TopoDS_Face BuildProfileFace(IIfcTrapeziumProfileDef^ trapeziumProfile);
				TopoDS_Face BuildProfileFace(IIfcTShapeProfileDef^ ifcTShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcUShapeProfileDef^ ifcUShapeProfileDef);
				TopoDS_Face BuildProfileFace(IIfcZShapeProfileDef^ ifcZShapeProfileDef);
				
				//constructs a rectProfilectangle wire with the bottom left corner at 0,0,0, top right at x,y,0
				TopoDS_Face BuildProfileFace(double x, double y, double tolerance, bool centre);
				TopoDS_Face BuildProfileFace(const TopoDS_Wire& wire);


				TopoDS_Edge BuildProfileEdge(IIfcProfileDef^ ifcProfileDef);
				TopoDS_Edge BuildProfileEdge(IIfcDerivedProfileDef^ ifcDerivedProfileDef);
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

#pragma region Build Curves from Profile Definition
			public:
				virtual IXCurve^ BuildCurve(IIfcProfileDef^ profileDef);
			internal:
				Handle(Geom_Curve) BuildCurve(IIfcProfileDef^ profileDef, XProfileDefType% curveType);
				Handle(Geom_Curve) BuildCurve(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef);

				Handle(Geom_Curve) BuildCurve(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef);

				Handle(Geom_Curve) BuildCurve(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef);

#pragma endregion
			protected:
				
			};
			
		}
	}
}

