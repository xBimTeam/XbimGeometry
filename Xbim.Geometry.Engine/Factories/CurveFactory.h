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


				void BuildSegments2d(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom2d_SequenceOfCurve& resultSegments, bool sameSense);

				template <typename IfcType>
				Handle(Geom2d_Curve) BuildCompositeCurveSegment2d(IfcType ifcCurve, bool sameSense);


				Handle(Geom2d_Curve) BuildCurve2d(IIfcCurve^ curve, XCurveType% curveType);

				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots);
				Handle(Geom2d_Circle) BuildCurve2d(IIfcCircle^ ifcCircle);
				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcCompositeCurve^ ifcCompositeCurve);

				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcCompositeCurveOnSurface^ ifcCompositeCurve);

				Handle(Geom2d_Ellipse) BuildCurve2d(IIfcEllipse^ ifcEllipse);

				Handle(Geom2d_LineWithMagnitude) BuildCurve2d(IIfcLine^ ifcLine);

				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve);

				Handle(TColGeom2d_SequenceOfBoundedCurve) GetIndexPolyCurveSegments2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve);

				Handle(Geom2d_OffsetCurve) BuildCurve2d(IIfcOffsetCurve2D^ ifcOffsetCurve2D);

				Handle(Geom2d_Curve) BuildCurve2d(IIfcPcurve^ ifcPcurve);

				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcPolyline^ ifcPolyline);

				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots);


				Handle(Geom_Curve) BuildCurve3d(IIfcCurve^ curve, XCurveType% curveType);
				Handle(Geom_BSplineCurve) BuildCurve3d(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots);
				Handle(Geom_Circle) BuildCurve3d(IIfcCircle^ ifcCircle);
				Handle(Geom_BSplineCurve) BuildCurve3d(IIfcCompositeCurve^ ifcCompositeCurve);
				Handle(Geom_BSplineCurve) BuildCurve3d(IIfcCompositeCurveOnSurface^ ifcCompositeCurve);
				Handle(Geom_Ellipse) BuildCurve3d(IIfcEllipse^ ifcEllipse);
				Handle(Geom_BSplineCurve) BuildCurve3d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve);
				Handle(TColGeom_SequenceOfBoundedCurve) GetIndexPolyCurveSegments3d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve);
				Handle(Geom_LineWithMagnitude) BuildCurve3d(IIfcLine^ ifcLine);
				Handle(Geom_OffsetCurve) CurveFactory::BuildCurve3d(IIfcOffsetCurve3D^ ifcOffsetCurve3D);


				
				virtual IXCurve^ BuildDirectrix(IIfcCurve^ curve, System::Nullable<double> startParam, System::Nullable<double> endParam);

				Handle(Geom_Curve) BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam, XCurveType% curveType);



				Handle(Geom_TrimmedCurve) BuildCurve3d(IIfcTrimmedCurve^ ifcTrimmedCurve);
				Handle(Geom2d_TrimmedCurve) BuildCurve2d(IIfcTrimmedCurve^ ifcTrimmedCurve);

				Handle(Geom_Curve) BuildCurve3d(IIfcPolyline^ ifcPolyline);
				Handle(Geom_Curve) BuildCurve3d(IIfcSurfaceCurve^ ifcPolyline);

				bool IsBoundedCurve(IIfcCurve^ curve);
			};

		}
	}
}