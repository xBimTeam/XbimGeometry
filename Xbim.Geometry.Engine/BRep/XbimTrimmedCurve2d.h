#pragma once
#include <Geom2d_TrimmedCurve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include "../XbimHandle.h"
using namespace Xbim::Common::Geometry;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimTrimmedCurve2d : XbimHandle<Handle(Geom2d_TrimmedCurve)>, IXTrimmedCurve
			{
			public:
				XbimTrimmedCurve2d(Handle(Geom2d_TrimmedCurve) hTrimmedCurve) : XbimHandle(new Handle(Geom2d_TrimmedCurve)(hTrimmedCurve)) {};
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcTrimmedCurve; }; };
				virtual property bool Is3d {bool get() { return false; }};
				virtual property IXCurve^ BasisCurve {IXCurve^ get(); };
				virtual property IXPoint^ StartPoint {IXPoint^ get(); };
				virtual property IXPoint^ EndPoint {IXPoint^ get(); };
				virtual  IXPoint^ GetPoint(double uParam);
				virtual  IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);
				virtual property double Length { double get() { return GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(OccHandle())); } }
			};
		}
	}
}