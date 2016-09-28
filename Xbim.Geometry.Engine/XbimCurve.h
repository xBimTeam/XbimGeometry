#pragma once
#include "XbimGeometryObject.h"
#include <Geom_Curve.hxx>

using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimCurve : IXbimCurve, XbimGeometryObject
		{
		private:
			IntPtr ptrContainer;
			virtual property Handle(Geom_Curve)* pCurve
			{
				Handle(Geom_Curve)* get() sealed { return (Handle(Geom_Curve)*)ptrContainer.ToPointer(); }
				void set(Handle(Geom_Curve)* val)sealed { ptrContainer = IntPtr(val); }
			}
			
			void InstanceCleanup();
			void Init(IIfcCurve^ curve);
			void Init(IIfcPolyline^ curve);
			void Init(IIfcCircle^ curve);
			void Init(IIfcEllipse^ curve);
			void Init(IIfcLine^ curve);
			void Init(IIfcTrimmedCurve^ curve);
			void Init(IIfcRationalBSplineCurveWithKnots^ curve);
			void Init(IIfcBSplineCurveWithKnots^ curve);
			void Init(IIfcOffsetCurve3D^ offset);
			void Init(IIfcPcurve^ curve);

		public:
			//destructors
			~XbimCurve(){ InstanceCleanup(); }
			!XbimCurve(){ InstanceCleanup(); }
			//constructors
			XbimCurve(const Handle(Geom_Curve)& curve);
			XbimCurve(IIfcCurve^ curve) { Init(curve); }
			XbimCurve(IIfcPolyline^ curve) {  Init(curve); }
			XbimCurve(IIfcCircle^ curve) {  Init(curve); }
			XbimCurve(IIfcEllipse^ curve) {  Init(curve); }
			XbimCurve(IIfcLine^ curve) {  Init(curve); }
			XbimCurve(IIfcTrimmedCurve^ curve) {  Init(curve); }
			XbimCurve(IIfcBSplineCurve^ curve) {  Init(curve); }
			XbimCurve(IIfcBSplineCurveWithKnots^ curve) {  Init(curve); }		
			XbimCurve(IIfcOffsetCurve3D^ curve){ Init(curve); }
			XbimCurve(IIfcPcurve^ curve){ Init(curve); }

#pragma region operators
			operator const Handle(Geom_Curve)& () { return *pCurve; }

#pragma endregion
			//properties
			virtual property bool IsValid{ bool get() override { return pCurve != nullptr; } }
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimCurveType; }; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual IEnumerable<XbimPoint3D>^ Intersections(IXbimCurve^ intersector, double tolerance);
			virtual property XbimPoint3D Start{XbimPoint3D get(); }
			virtual property XbimPoint3D End{XbimPoint3D get(); }
			virtual property double Length{double get(); }
			virtual double GetParameter(XbimPoint3D point, double tolerance);
			virtual XbimPoint3D GetPoint(double parameter);
			virtual property bool IsClosed{ bool get(); }
			virtual property bool Is3D{ bool get(){ return true; }; }
			virtual property double FirstParameter{double get(){ return IsValid ? (*pCurve)->FirstParameter() : 0; }; }
			virtual property double LastParameter{double get(){ return IsValid ? (*pCurve)->LastParameter() : 0; }; }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() override; }
		};
	}
}

