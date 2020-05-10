#pragma once
#include <Geom_Line.hxx>
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
			

			public ref class XbimLine : XbimHandle<Handle(Geom_Line)>, IXLine
			{
			private:
				double parametricUnit;				
			public:				
				XbimLine(Handle(Geom_Line) hLine, double pUnit) : XbimHandle(new Handle(Geom_Line)(hLine)), parametricUnit(pUnit) {};
				XbimLine(Handle(Geom_Line) pLine) : XbimHandle(new Handle(Geom_Line)(pLine)), parametricUnit(1.0) {};
				virtual property XCurveType CurveType {XCurveType get() {return XCurveType::Line;}; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return parametricUnit; };};
				virtual property bool Is3d {bool get() {return true; }};
				//Get a Point at the parameter length from the origin
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^ %normal);
			};
		}
	}
}


