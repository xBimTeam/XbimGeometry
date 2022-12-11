#pragma once
#include "../XbimHandle.h"
#include <Geom_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCurve abstract : XbimHandle<Handle(Geom_Curve)>, IXCurve
			{
			public:
				XCurve(Handle(Geom_Curve) curve) : XbimHandle(new Handle(Geom_Curve)(curve))
				{
				}

				static const Handle(Geom_Curve) GeomCurve(IXCurve^ xCurve);
				static  IXCurve^ GeomToXCurve(Handle(Geom_Curve) curve);
				property XCurveType CurveType {virtual XCurveType get()  abstract; }
				
				property bool Is3d {virtual bool get() { return true; }; }
				virtual property double FirstParameter {double get() 
				{
					return OccHandle()->FirstParameter();
				}};

				virtual property double LastParameter {double get() 
				{
					return OccHandle()->LastParameter();
				}};
				virtual IXPoint^ GetPoint(double uParam) ;
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal) ;
				virtual property double Length { double get()  { return GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(OccHandle())); } }
			};
		}
	}
}

