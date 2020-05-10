#pragma once
#include <Geom_TrimmedCurve.hxx>
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
			public ref class XbimTrimmedCurve : XbimHandle<Handle(Geom_TrimmedCurve)>, IXTrimmedCurve
			{
			public:
				XbimTrimmedCurve(Handle(Geom_TrimmedCurve) hTrimmedCurve) : XbimHandle(new Handle(Geom_TrimmedCurve)(hTrimmedCurve)) {};
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::TrimmedCurve; }; };
				virtual property bool Is3d {bool get() { return true; }};
				virtual property IXCurve^ BasisCurve {IXCurve^ get(); };
				virtual property IXPoint^ StartPoint {IXPoint^ get(); };
				virtual property IXPoint^ EndPoint {IXPoint^ get(); };
				virtual  IXPoint^ GetPoint(double uParam);
				virtual  IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);

			};
		}
	}
}
