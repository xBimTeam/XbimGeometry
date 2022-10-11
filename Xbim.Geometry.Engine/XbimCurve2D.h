#pragma once

#include <Geom2d_Curve.hxx>
#include "XbimGeometryObject.h"
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
using namespace Microsoft::Extensions::Logging;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimCurve2DV5 : IXbimCurve, XbimGeometryObject
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
			XbimCurve2DV5(const Handle(Geom2d_Curve)& curve2d);
			XbimCurve2DV5(const Handle(Geom2d_Curve)& curve2d, double p1, double p2);
			
			//destructors
			~XbimCurve2DV5(){ InstanceCleanup(); }
			!XbimCurve2DV5(){ InstanceCleanup(); }

		    XbimCurve2DV5(IIfcGridAxis^ axis, ILogger^ logger) { Init(axis,logger); }
			XbimCurve2DV5(IIfcCurve^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcPolyline^ curv, ILogger^ logger) { Init(curv, logger); }
			XbimCurve2DV5(IIfcCircle^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcEllipse^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcLine^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcTrimmedCurve^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcBSplineCurve^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcBSplineCurveWithKnots^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve2DV5(IIfcOffsetCurve2D^ curve, ILogger^ logger){ Init(curve, logger); }

#pragma region operators
			operator const Handle(Geom2d_Curve)& () { return *pCurve2D; }
			
#pragma endregion
			//properties
			virtual property bool IsValid{ bool get() override { return pCurve2D != nullptr; } }
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimCurveType; }; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual IEnumerable<XbimPoint3D>^ Intersections(IXbimCurve^ intersector,double tolerance, ILogger^logger);
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

