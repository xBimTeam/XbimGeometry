#pragma once
#include "../BRep/OccExtensions/Geom_LineWithMagnitude.h"	
#include "XCurve.h"
#include "../XbimHandle.h"
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
using namespace Xbim::Common::Geometry;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;

#define OccLine() Handle(Geom_Line)::DownCast(this->Ref())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			
			public ref class XLine : XCurve, IXLine
			{
					
			public:				
				XLine(Handle(Geom_Line) hLine) : XCurve(hLine) {};
				virtual property XCurveType CurveType {XCurveType get() override {return XCurveType::IfcLine;}; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get(); };
			};
		}
	}
}


