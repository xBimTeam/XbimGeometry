#pragma once
#include "../XbimHandle.h"
#include <Geom2d_Curve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>

using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCurve2d abstract : XbimHandle<Handle(Geom2d_Curve)>, IXCurve
			{
			public:
				XCurve2d(Handle(Geom2d_Curve) curve) : XbimHandle(new Handle(Geom2d_Curve)(curve))
				{
				}
				property XCurveType CurveType {virtual XCurveType get()  abstract; }

				property bool Is3d {virtual bool get() { return false; }; }
				virtual property double FirstParameter {double get()
				{
					return OccHandle()->FirstParameter();
				}};

				virtual property double LastParameter {double get()
				{
					return OccHandle()->LastParameter();
				}};
				static const Handle(Geom2d_Curve) GeomCurve2d(IXCurve^ xCurve);
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);
				virtual property double Length { double get() { return GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(OccHandle())); } };
			};
		}
	}
}

