#pragma once
#include "../XbimHandle.h"
#include "XCurve.h"
#include "XPoint.h"
#include <Geom_BSplineCurve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>


using namespace Xbim::Geometry::Abstractions;
#define OccBSplineCurve() Handle(Geom_BSplineCurve)::DownCast(this->Ref())

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XBSplineCurve :XCurve, IXBSplineCurve
			{
			public:
				XBSplineCurve(Handle(Geom_BSplineCurve) hSpline) : XCurve(hSpline, XCurveType::IfcCompositeCurve) { };
				
				virtual property bool IsPeriodic {bool get() { return OccBSplineCurve()->IsPeriodic(); } }
				virtual property bool IsRational {bool get() { return OccBSplineCurve()->IsRational(); } }				
				virtual property IXPoint^ StartPoint {IXPoint^ get() { return gcnew XPoint(OccBSplineCurve()->StartPoint()); }};
				virtual property IXPoint^ EndPoint {IXPoint^ get() { return gcnew XPoint(OccBSplineCurve()->EndPoint()); }};
				virtual property XGeometricContinuity Continuity {XGeometricContinuity get()
				{
					GeomAbs_Shape continuity = OccBSplineCurve()->Continuity();
					switch (continuity)
					{
						case GeomAbs_C0:
							return XGeometricContinuity::GeomAbs_C0;
						case GeomAbs_G1:
							return XGeometricContinuity::GeomAbs_G1;
						case GeomAbs_C1:
							return XGeometricContinuity::GeomAbs_C1;
						case GeomAbs_G2:
							return XGeometricContinuity::GeomAbs_G2;
						case GeomAbs_C2:
							return XGeometricContinuity::GeomAbs_C2;
						case GeomAbs_C3:
							return XGeometricContinuity::GeomAbs_C3;
						case GeomAbs_CN:
							return XGeometricContinuity::GeomAbs_CN;
						default:
							return XGeometricContinuity::GeomAbs_C0;
					}
				};

				};
				
			};
		}
	}
}

