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
			public ref class XbimCompositeCurve :XbimHandle<Handle(Geom_BSplineCurve)>, IXCompositeCurve
			{
			public:
				XbimCompositeCurve(Handle(Geom_BSplineCurve) hSpline) : XbimHandle(new Handle(Geom_BSplineCurve)(hSpline)) { };
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

				virtual property double Length {double get() { return GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(OccHandle()));}};
				virtual property IXPoint^ StartPoint {IXPoint^ get() { return gcnew XbimPoint(OccHandle()->StartPoint()); }};
				virtual property IXPoint^ EndPoint {IXPoint^ get() { return gcnew XbimPoint(OccHandle()->EndPoint()); }};
				virtual property int NumberOfSegments {int get() { return OccHandle()->NbKnots()-1; }};
			};
		}
	}
}

