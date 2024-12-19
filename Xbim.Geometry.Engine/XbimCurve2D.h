#pragma once

#include <Geom2d_Curve.hxx>
#include "XbimGeometryObject.h"
#include <Geom2d_TrimmedCurve.hxx>
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
using namespace Microsoft::Extensions::Logging;
using namespace Xbim::Geometry::Services;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimCurve2D : IXbimCurve, XbimGeometryObject
		{
		private:
			System::IntPtr ptrContainer;
			virtual property Handle(Geom2d_Curve)* pCurve2D
			{
				Handle(Geom2d_Curve)* get() sealed { return (Handle(Geom2d_Curve)*)ptrContainer.ToPointer(); }
				void set(Handle(Geom2d_Curve)* val)sealed { ptrContainer = System::IntPtr(val); }
			}
			void InstanceCleanup();
			void Init(IIfcGridAxis^ axis, ILogger^ logger);
			void Init(IIfcCurve^ curve, ILogger^ logger);
			void Init(IIfcPolyline^ curve, ILogger^ logger);
			void Init(IIfcCircle^ curve, ILogger^ logger);
			void Init(IIfcEllipse^ curve, ILogger^ logger);
			void Init(IIfcLine^ curve, ILogger^ logger);
			void Init(IIfcIndexedPolyCurve ^ curve, ILogger ^);
			void Init(IIfcTrimmedCurve^ curve, ILogger^ logger);
			void Init(IIfcRationalBSplineCurveWithKnots^ curve, ILogger^ logger);
			void Init(IIfcBSplineCurveWithKnots^ curve, ILogger^ logger);
			void Init(IIfcOffsetCurve2D^ offset, ILogger^ logger);
			void Init(IIfcPcurve^ curve, ILogger^ logger);
		public:
			XbimCurve2D(const Handle(Geom2d_Curve)& curve2d, ModelGeometryService^ modelService) :XbimGeometryObject(modelService)
			{
				this->pCurve2D = new Handle(Geom2d_Curve);
				*pCurve2D = curve2d;
			};
			XbimCurve2D(const Handle(Geom2d_Curve)& curve2d, double p1, double p2, ModelGeometryService^ modelService) :XbimGeometryObject(modelService)
			{
				this->pCurve2D = new Handle(Geom2d_Curve);
				*pCurve2D = new Geom2d_TrimmedCurve(curve2d, p1, p2, true);
			}
			
			//destructors
			~XbimCurve2D(){ InstanceCleanup(); }
			!XbimCurve2D(){ InstanceCleanup(); }

		    XbimCurve2D(IIfcGridAxis^ axis, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(axis,logger); }
			XbimCurve2D(IIfcCurve^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcPolyline^ curv, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curv, logger); }
			XbimCurve2D(IIfcCircle^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcEllipse^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcLine^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcTrimmedCurve^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcBSplineCurve^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcBSplineCurveWithKnots^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }
			XbimCurve2D(IIfcOffsetCurve2D^ curve, ILogger^ logger, ModelGeometryService^ modelService) :XbimGeometryObject(modelService) { Init(curve, logger); }

#pragma region operators
			operator const Handle(Geom2d_Curve)& () { return *pCurve2D; }
			
#pragma endregion
			//properties
			virtual property bool IsValid{ bool get() override { return pCurve2D != nullptr; } }
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimCurveType; }; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual System::Collections::Generic::IEnumerable<XbimPoint3D>^ Intersections(IXbimCurve^ intersector, double tolerance, ILogger^ logger);
			virtual property XbimPoint3D Start{XbimPoint3D get(); }
			virtual property XbimPoint3D End{XbimPoint3D get(); }
			virtual property double Length{double get(); }
			virtual double GetParameter(XbimPoint3D point, double tolerance);
			virtual XbimPoint3D GetPoint(double parameter);
			virtual property bool IsClosed{ bool get(); }
			virtual property bool Is3D{ bool get(){ return true; }; }
			virtual XbimVector3D TangentAt(double parameter);
			IXbimCurve^ ToCurve3D();
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
		};
	}
}

