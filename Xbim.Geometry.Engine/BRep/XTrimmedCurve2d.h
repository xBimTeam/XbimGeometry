#pragma once
#include "XCurve2d.h"
#include <Geom2d_TrimmedCurve.hxx>
#define OccTrimmedCurve2d() Handle(Geom2d_TrimmedCurve)::DownCast(this->Ref())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XTrimmedCurve2d : XCurve2d, IXTrimmedCurve
			{
			public:
				XTrimmedCurve2d(Handle(Geom2d_TrimmedCurve) hTrimmedCurve) : XCurve2d(hTrimmedCurve, XCurveType::IfcTrimmedCurve) {};
		
				virtual property IXCurve^ BasisCurve {IXCurve^ get(); };
				virtual property IXPoint^ StartPoint {IXPoint^ get(); };
				virtual property IXPoint^ EndPoint {IXPoint^ get(); };

			};
		}
	}
}