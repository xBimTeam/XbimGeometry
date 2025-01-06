#pragma once
#include "../XbimHandle.h"
#include <Geom2d_Curve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>

using namespace Xbim::Geometry::Abstractions;

using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCurve2d  : XbimHandle<Handle(Geom2d_Curve)>, IXCurve
			{
			private:
				XCurveType _curveType = XCurveType::IfcCurve;
			public:
				XCurve2d(Handle(Geom2d_Curve) curve, XCurveType curveType) : XbimHandle(new Handle(Geom2d_Curve)(curve))
				{
					_curveType = curveType;
				}			
				virtual property XCurveType CurveType {XCurveType get() { return _curveType; }; };
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
				virtual IXPoint^ GetFirstDerivative(double uParam, [System::Runtime::InteropServices::Out] IXDirection^% direction);
				virtual IXPoint^ GetSecondDerivative(double uParam, [System::Runtime::InteropServices::Out] IXDirection^% direction, [System::Runtime::InteropServices::Out] IXDirection^% normal);
				virtual property double Length { double get() { return 0;/*GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(OccHandle()));*/ } };
			};
		}
	}
}

