#pragma once
#include "./Unmanaged/NWireFactory.h"
#include "FactoryBase.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class WireFactory : FactoryBase<NWireFactory>, IXWireFactory
			{
			private:
				
			internal:
				//The distance between two points at which they are determined to be equal points
				
				TopoDS_Wire BuildWire(IIfcCurve^ ifcCurve, bool asSingleEdge);

				
				TopoDS_Wire BuildWire2d(IIfcCurve^ ifcCurve, bool asSingleEdge);
				TopoDS_Wire BuildWire(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots);
				TopoDS_Wire BuildWire(IIfcCircle^ ifcCircle);
				TopoDS_Wire BuildWire2d(IIfcCompositeCurve^ ifcCompositeCurve, bool asSingleEdge);
				TopoDS_Wire BuildWire(IIfcEllipse^ ifcEllipse);
				TopoDS_Wire BuildWire2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, bool asSingleEdge);
				TopoDS_Wire BuildWire(IIfcLine^ ifcLine);
				TopoDS_Wire BuildWire2d(IIfcOffsetCurve2D^ ifcOffsetCurve2D, bool asSingleEdge);
				TopoDS_Wire BuildWire2d(IIfcPolyline^ ifcPolyline, bool asSingleEdge);
				TopoDS_Wire BuildWire(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots);
				TopoDS_Wire BuildWire(IIfcTrimmedCurve^ ifcTrimmedCurve, bool asSingleEdge);

				TopoDS_Wire BuildWire3d(IIfcCurve^ ifcCurve, bool asSingleEdge);
				TopoDS_Wire BuildWire3d(IIfcCompositeCurve^ ifcCompositeCurve, bool asSingleEdge);

				TopoDS_Wire BuildWire3d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, bool asSingleEdge);

				
				TopoDS_Wire BuildDirectrixWire(IIfcCurve^ ifcCurve, double startParam, double endParam);
				//void AdjustDirectrixTrimParameters(IIfcCurve^ ifcCurve, Nullable<IfcParameterValue> startParam, Nullable<IfcParameterValue> endParam, double& start, double& end);
			public:
				WireFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NWireFactory()) {}				
				virtual IXWire^ BuildWire(array<IXPoint^>^  points);
				virtual IXWire^ Build(IIfcCurve^ ifcCurve);
				virtual IXWire^ Build(IIfcProfileDef^ ifcProfileDef);
				
			};

		}
	}
}

