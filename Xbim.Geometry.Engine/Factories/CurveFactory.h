#pragma once
#include "./Unmanaged/NCurveFactory.h"
#include "FactoryBase.h"

#include <TColGeom2d_SequenceOfBoundedCurve.hxx>
#include <TColGeom_SequenceOfBoundedCurve.hxx>
using namespace Xbim::Ifc4::MeasureResource;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class CurveFactory : FactoryBase<NCurveFactory>, IXCurveFactory
			{

			public:
				CurveFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NCurveFactory())
				{

				}

#pragma region Top level abstraction for building any XCurve
			public:
				virtual IXCurve^ Build(IIfcCurve^ curve);
				virtual IXCurve^ BuildDirectrix(IIfcCurve^ curve, System::Nullable<double> startParam, System::Nullable<double> endParam);

			protected:
				virtual IXCurve^ BuildXCurve(Handle(Geom_Curve) curve, XCurveType curveType);
				virtual IXCurve^ BuildXCurve(Handle(Geom2d_Curve) curve, XCurveType curveType);
				IXCurve^ BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam);
			
#pragma endregion
#pragma region Build Curves (3D)
			public:
				Handle(Geom_Curve) BuildCurve(IIfcCurve^ curve);
				Handle(Geom_Curve) BuildCurve(IIfcCurve^ curve, XCurveType% curveType);
				Handle(Geom_BSplineCurve) BuildCurve(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots);
				Handle(Geom_Circle) BuildCurve(IIfcCircle^ ifcCircle);
				Handle(Geom_BSplineCurve) BuildCurve(IIfcCompositeCurve^ ifcCompositeCurve);
				Handle(Geom_BSplineCurve) BuildCurve(IIfcCompositeCurveOnSurface^ ifcCompositeCurveOnSurface);
				Handle(Geom_Ellipse) BuildCurve(IIfcEllipse^ ifcEllipse);
				Handle(Geom_BSplineCurve) BuildCurve(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve);
				Handle(Geom_LineWithMagnitude) BuildCurve(IIfcLine^ ifcLine);
				Handle(Geom_OffsetCurve) BuildCurve(IIfcOffsetCurve2D^ ifcOffsetCurve2D);
				Handle(Geom_OffsetCurve) CurveFactory::BuildCurve(IIfcOffsetCurve3D^ ifcOffsetCurve3D);
				Handle(Geom_Curve) BuildCurve(IIfcPcurve^ ifcPcurve);
				Handle(Geom_Curve) BuildCurve(IIfcPolyline^ ifcPolyline);
				Handle(Geom_BSplineCurve) BuildCurve(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots);
				Handle(Geom_Curve) BuildCurve(IIfcSurfaceCurve^ ifcPolyline);
				Handle(Geom_TrimmedCurve) BuildCurve(IIfcTrimmedCurve^ ifcTrimmedCurve);
#pragma endregion

#pragma region Build Curves (2D)
				Handle(Geom2d_Curve) BuildCurve2d(IIfcCurve^ curve);
				Handle(Geom2d_Curve) BuildCurve2d(IIfcCurve^ curve, XCurveType% curveType);
				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots);
				Handle(Geom2d_Circle) BuildCurve2d(IIfcCircle^ ifcCircle);
				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcCompositeCurve^ ifcCompositeCurve);
				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcCompositeCurveOnSurface^ ifcCompositeCurve);
				Handle(Geom2d_Ellipse) BuildCurve2d(IIfcEllipse^ ifcEllipse);
				Handle(Geom2d_LineWithMagnitude) BuildCurve2d(IIfcLine^ ifcLine);
				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve);
				Handle(Geom2d_OffsetCurve) BuildCurve2d(IIfcOffsetCurve2D^ ifcOffsetCurve2D);
				Handle(Geom2d_Curve) BuildCurve2d(IIfcPcurve^ ifcPcurve);
				Handle(Geom2d_Curve) BuildCurve2d(IIfcPolyline^ ifcPolyline);
				Handle(Geom2d_BSplineCurve) BuildCurve2d(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots);
				Handle(Geom2d_Curve) BuildCurve2d(IIfcSurfaceCurve^ ifcPolyline);
				Handle(Geom2d_TrimmedCurve) BuildCurve2d(IIfcTrimmedCurve^ ifcTrimmedCurve);

#pragma endregion

#pragma region Helpers
				template <typename IfcType>
				Handle(Geom_Curve) BuildCompositeCurveSegment3d(IfcType ifcCurve, bool sameSense);
				template <typename IfcType>
				Handle(Geom2d_Curve) BuildCompositeCurveSegment2d(IfcType ifcCurve, bool sameSense);
				void BuildPolylineSegments3d(IIfcPolyline^ ifcPolyline, TColGeom_SequenceOfBoundedCurve& segments);
				void BuildPolylineSegments2d(IIfcPolyline^ ifcPolyline, TColGeom2d_SequenceOfBoundedCurve& segments);
				void BuildIndexPolyCurveSegments3d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, TColGeom_SequenceOfBoundedCurve& segments);
				void BuildCompositeCurveSegments3d(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom_SequenceOfBoundedCurve& segments);
				Handle(Geom2d_TrimmedCurve) BuildLinearSegment(const gp_Pnt2d& start, const gp_Pnt2d& end);
				Handle(Geom_TrimmedCurve) BuildLinearSegment(const gp_Pnt& start, const gp_Pnt& end);
				Handle(Geom_Curve) BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam, XCurveType% curveType);
				Handle(Geom_Curve) BuildDirectrixCurve(IIfcCurve^ curve, System::Nullable<IfcParameterValue> startParam, System::Nullable<IfcParameterValue> endParam);
				bool IsBoundedCurve(IIfcCurve^ curve);
				bool Tangent2dAt(const Handle(Geom2d_Curve)& curve, double parameter, gp_Pnt2d& pnt2d, gp_Vec2d& tangent);
				void BuildIndexPolyCurveSegments2d(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, TColGeom2d_SequenceOfBoundedCurve& segments);
				void BuildCompositeCurveSegments2d(IIfcCompositeCurve^ ifcCompositeCurve, TColGeom2d_SequenceOfBoundedCurve& segments);
				Handle(Geom2d_Curve) BuildAxis2d(IIfcGridAxis^ axis);
				int Intersections(const Handle(Geom2d_Curve)& c1, const Handle(Geom2d_Curve)& c2, TColgp_Array1OfPnt2d& intersections);
#pragma endregion
			};

		}
	}
}