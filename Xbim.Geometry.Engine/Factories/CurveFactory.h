#pragma once
#include "./Unmanaged/NCurveFactory.h"
#include "FactoryBase.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class CurveFactory : FactoryBase<NCurveFactory>, IXCurveFactory
			{
			
			public:
				CurveFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NCurveFactory()) {}

				//Top level abstraction for building any curve

				IXCurve^ BuildXCurve(Handle(Geom_Curve) curve, XCurveType curveType);
				IXCurve^ BuildXCurve(Handle(Geom2d_Curve) curve, XCurveType curveType);
				IXCurve^ BuildXDirectrix(IIfcCurve^ curve, double startParam, double endParam);

				//Geometry builders
				virtual IXCurve^ Build(IIfcCurve^ curve);


				Handle(Geom_Curve) BuildCurve3d(IIfcCurve^ curve, XCurveType% curveType);
				Handle(Geom2d_Curve) BuildCurve2d(IIfcCurve^ curve, XCurveType% curveType);


virtual IXCurve^ BuildDirectrix(IIfcCurve^ curve, System::Nullable<double> startParam, System::Nullable<double> endParam);
				/*Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcBoundaryCurve^ ifcBoundaryCurve);
				Handle(Geom2d_TrimmedCurve) BuildCurve2d(IIfcBoundaryCurve^ ifcBoundaryCurve);*/

				Handle(Geom_Curve) BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam, XCurveType% curveType);
				/*Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcBoundaryCurve^ ifcBoundaryCurve);
				Handle(Geom2d_TrimmedCurve) BuildCurve2d(IIfcBoundaryCurve^ ifcBoundaryCurve);*/

				Handle(Geom_LineWithMagnitude) BuildGeom3d(IIfcLine^ ifcLine);
				Handle(Geom2d_LineWithMagnitude) BuildCurve2d(IIfcLine^ ifcLine);

				Handle(Geom_Circle) BuildGeom3d(IIfcCircle^ ifcCircle);
				Handle(Geom2d_Circle) BuildCurve2d(IIfcCircle^ ifcCircle);

				Handle(Geom_Ellipse) BuildGeom3d(IIfcEllipse^ ifcEllipse);
				Handle(Geom2d_Ellipse) BuildCurve2d(IIfcEllipse^ ifcEllipse);

				Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcTrimmedCurve^ ifcTrimmedCurve);
				Handle(Geom2d_TrimmedCurve) BuildCurve2d(IIfcTrimmedCurve^ ifcTrimmedCurve);

				Handle(Geom_Curve) BuildGeom3d(IIfcPolyline^ ifcPolyline);

				//Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcPolyline^ ifcPolyline);

				Handle(Geom_BSplineCurve) BuildGeom3d(IIfcCompositeCurve^ ifcCompositeCurve);


				bool IsBoundedCurve(IIfcCurve^ curve);
			};

		}
	}
}