#pragma once
#include "XbimGeometryObject.h"
#include <Geom2d_Curve.hxx>
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimCurve2D : IXbimCurve, XbimGeometryObject
		{
		private:
			IntPtr ptrContainer;
			virtual property Handle(Geom2d_Curve)* pCurve2D
			{
				Handle(Geom2d_Curve)* get() sealed { return (Handle(Geom2d_Curve)*)ptrContainer.ToPointer(); }
				void set(Handle(Geom2d_Curve)* val)sealed { ptrContainer = IntPtr(val); }
			}
			void InstanceCleanup();
			void Init(IIfcGridAxis^ axis);
			void Init(IIfcCurve^ curve);
			void Init(IIfcPolyline^ curve);
			void Init(IIfcCircle^ curve);
			void Init(IIfcEllipse^ curve);
			void Init(IIfcLine^ curve);
			void Init(IIfcTrimmedCurve^ curve);
			void Init(IIfcRationalBSplineCurveWithKnots^ curve);
			void Init(IIfcBSplineCurveWithKnots^ curve);
			void Init(IIfcOffsetCurve2D^ offset);
		public:
			XbimCurve2D(const Handle(Geom2d_Curve)& curve2d);
			XbimCurve2D(const Handle(Geom2d_Curve)& curve2d, double p1, double p2);
			
			//destructors
			~XbimCurve2D(){ InstanceCleanup(); }
			!XbimCurve2D(){ InstanceCleanup(); }

		    XbimCurve2D(IIfcGridAxis^ axis) { Init(axis); }
			XbimCurve2D(IIfcCurve^ curve) { Init(curve); }
			XbimCurve2D(IIfcPolyline^ curve) { Init(curve); }
			XbimCurve2D(IIfcCircle^ curve) { Init(curve); }
			XbimCurve2D(IIfcEllipse^ curve) { Init(curve); }
			XbimCurve2D(IIfcLine^ curve) { Init(curve); }
			XbimCurve2D(IIfcTrimmedCurve^ curve) { Init(curve); }
			XbimCurve2D(IIfcBSplineCurve^ curve) { Init(curve); }
			XbimCurve2D(IIfcBSplineCurveWithKnots^ curve) { Init(curve); }
			XbimCurve2D(IIfcOffsetCurve2D^ curve){ Init(curve); }

#pragma region operators
			operator const Handle(Geom2d_Curve)& () { return *pCurve2D; }
			
#pragma endregion
			//properties
			virtual property bool IsValid{ bool get() override { return pCurve2D != nullptr; } }
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimCurveType; }; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual IEnumerable<XbimPoint3D>^ Intersections(IXbimCurve^ intersector,double tolerance);
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

