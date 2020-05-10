#pragma once
#include <Geom2d_Line.hxx>
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

			public ref class Xbim2dLine : XbimHandle<Handle(Geom2d_Line)>, IXLine
			{
			private:
				double parametricUnit;				
			public:
				
				Xbim2dLine(Handle(Geom2d_Line) hLine, double pUnit) : XbimHandle(new Handle(Geom2d_Line)(hLine)), parametricUnit(pUnit) {};
				Xbim2dLine(Handle(Geom2d_Line) hLine) : XbimHandle(new Handle(Geom2d_Line)(hLine)), parametricUnit(1.0) {};
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::Line2D; }; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return parametricUnit; }; };
				virtual property bool Is3d {bool get() { return false; }};
				//Get a Point at the parameter length from the origin
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);
			};
		}
	}
}


