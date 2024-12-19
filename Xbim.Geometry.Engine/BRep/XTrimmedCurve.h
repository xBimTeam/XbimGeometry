#pragma once
#include "XCurve.h"
#include <Geom_TrimmedCurve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
#include "../XbimHandle.h"

#define OccTrimmedCurve() Handle(Geom_TrimmedCurve)::DownCast(this->Ref())

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{			
			public ref class XTrimmedCurve : XCurve, IXTrimmedCurve
			{
			public:
				XTrimmedCurve(Handle(Geom_TrimmedCurve) hTrimmedCurve) : XCurve(hTrimmedCurve, XCurveType::IfcTrimmedCurve) {};
					
				virtual property IXCurve^ BasisCurve {IXCurve^ get(); };
				virtual property IXPoint^ StartPoint {IXPoint^ get(); };
				virtual property IXPoint^ EndPoint {IXPoint^ get(); };
			};
		}
	}
}
