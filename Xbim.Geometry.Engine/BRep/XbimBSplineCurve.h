#pragma once
#include "../XbimHandle.h"
#include "XbimPoint.h"
#include <Geom_BSplineCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>

using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimBSplineCurve :XbimHandle<Handle(Geom_BSplineCurve)>, IXBSplineCurve
			{
			public:
				XbimBSplineCurve(Handle(Geom_BSplineCurve) hSpline) : XbimHandle(new Handle(Geom_BSplineCurve)(hSpline)) { };
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcCompositeCurve; }; };
				virtual property bool Is3d {bool get() { return true; }};
				virtual property double FirstParameter {double get()
				{
					return OccHandle()->FirstParameter();
				}};

				virtual property double LastParameter {double get()
				{
					return OccHandle()->LastParameter();
				}};

				virtual property double Length {double get() { return GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(OccHandle())); }};
				virtual property IXPoint^ StartPoint {IXPoint^ get() { return gcnew XbimPoint(OccHandle()->StartPoint()); }};
				virtual property IXPoint^ EndPoint {IXPoint^ get() { return gcnew XbimPoint(OccHandle()->EndPoint()); }};
				virtual property IXGeometricContinuity Continuity {IXGeometricContinuity get()
				{
					GeomAbs_Shape continuity = OccHandle()->Continuity();
					switch (continuity)
					{
					case GeomAbs_C0:
						return IXGeometricContinuity::GeomAbs_C0;
					case GeomAbs_G1:
						return IXGeometricContinuity::GeomAbs_G1;
					case GeomAbs_C1:
						return IXGeometricContinuity::GeomAbs_C1;
					case GeomAbs_G2:
						return IXGeometricContinuity::GeomAbs_G2;
					case GeomAbs_C2:
						return IXGeometricContinuity::GeomAbs_C2;
					case GeomAbs_C3:
						return IXGeometricContinuity::GeomAbs_C3;
					case GeomAbs_CN:
						return IXGeometricContinuity::GeomAbs_CN;
					default:
						return IXGeometricContinuity::GeomAbs_C0;
					}
				};
				};

			};
		}
	}
}

