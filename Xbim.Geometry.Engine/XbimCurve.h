#pragma once
#include "XbimGeometryObject.h"
#include <Geom_Curve.hxx>

#include <Handle_Geom_Curve.hxx>

using namespace XbimGeometry::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimCurve : IXbimCurve, XbimGeometryObject
		{
		private:
			IntPtr ptrContainer;
			virtual property Handle_Geom_Curve* pCurve
			{
				Handle_Geom_Curve* get() sealed { return (Handle_Geom_Curve*)ptrContainer.ToPointer(); }
				void set(Handle_Geom_Curve* val)sealed { ptrContainer = IntPtr(val); }
			}
			double startParam;
			double endParam;
			void InstanceCleanup();
		public:
			//destructors
			~XbimCurve(){ InstanceCleanup(); }
			!XbimCurve(){ InstanceCleanup(); }
			//constructors
			XbimCurve(const Handle_Geom_Curve& curve, double p1, double p2);

			//properties
			virtual property bool IsValid{ bool get() override { return pCurve != nullptr; } }
			virtual property bool IsSet{bool get() override { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() override { return XbimGeometryObjectType::XbimCurveType; }; }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) override;
		};
	}
}

