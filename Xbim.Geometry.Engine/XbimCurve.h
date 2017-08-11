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
			void Init(IIfcCurve^ curve, ILogger^ logger);
			void Init(IIfcPolyline^ curve, ILogger^ logger);
			void Init(IIfcCircle^ curve, ILogger^ logger);
			void Init(IIfcEllipse^ curve, ILogger^ logger);
			void Init(IIfcLine^ curve, ILogger^ logger);
			void Init(IIfcTrimmedCurve^ curve, ILogger^ logger);
			void Init(IIfcRationalBSplineCurveWithKnots^ curve, ILogger^ logger);
			void Init(IIfcBSplineCurveWithKnots^ curve, ILogger^ logger);
			void Init(IIfcOffsetCurve3D^ offset, ILogger^ logger);
			void Init(IIfcPcurve^ curve, ILogger^ logger);

		public:
			//destructors
			~XbimCurve(){ InstanceCleanup(); }
			!XbimCurve(){ InstanceCleanup(); }
			//constructors
			XbimCurve(const Handle(Geom_Curve)& curve);
			XbimCurve(IIfcCurve^ curve, ILogger^ logger) { Init(curve, logger); }
			XbimCurve(IIfcPolyline^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcCircle^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcEllipse^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcLine^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcTrimmedCurve^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcBSplineCurve^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcBSplineCurveWithKnots^ curve, ILogger^ logger) {  Init(curve, logger); }
			XbimCurve(IIfcOffsetCurve3D^ curve, ILogger^ logger){ Init(curve, logger); }
			XbimCurve(IIfcPcurve^ curve, ILogger^ logger){ Init(curve, logger); }

#pragma region operators
			operator const Handle(Geom_Curve)& () { return *pCurve; }

#pragma endregion
			//properties
			virtual property bool IsValid{ bool get() override { return pCurve != nullptr; } }
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimCurveType; }; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D)override;
			virtual IEnumerable<XbimPoint3D>^ Intersections(IXbimCurve^ intersector, double tolerance, ILogger^ logger);
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

