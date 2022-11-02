#pragma once
#include "../BRep/OccExtensions/Geom2d_LineWithMagnitude.h"	
#include "XCurve2d.h"

#define OccLine2d() Handle(Geom2d_LineWithMagnitude)::DownCast(this->Ref())

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XLine2d : XCurve2d, IXLine
			{
					
			public:
				
				XLine2d(Handle(Geom2d_LineWithMagnitude) hLine) : XCurve2d(hLine) {};
				virtual property XCurveType CurveType {XCurveType get() override { return XCurveType::IfcLine; }; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return OccLine2d()->Magnitude(); }; };
				
			};
		}
	}
}


