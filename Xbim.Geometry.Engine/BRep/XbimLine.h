#pragma once
#include "../BRep/OccExtensions/Geom_LineWithMagnitude.h"		
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
			

			public ref class XbimLine : XbimHandle<Handle(Geom_LineWithMagnitude)>, IXLine
			{
					
			public:				
				XbimLine(Handle(Geom_LineWithMagnitude) hLine) : XbimHandle(new Handle(Geom_LineWithMagnitude)(hLine)) {};
				virtual property XCurveType CurveType {XCurveType get() {return XCurveType::IfcLine;}; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return OccHandle()->Magnitude(); };};
				virtual property bool Is3d {bool get() {return true; }};
				//Get a Point at the parameter length from the origin
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^ %normal);
			};
		}
	}
}


