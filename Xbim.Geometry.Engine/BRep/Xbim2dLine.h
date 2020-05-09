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

			public ref class Xbim2dLine : XbimHandle<Geom2d_Line>, IXLine
			{
			private:
				double parametricUnit;
				bool is3d = false;
			public:
				
				Xbim2dLine(Geom2d_Line* pLine, double pUnit) : XbimHandle(pLine), parametricUnit(pUnit), is3d(false) {};
				Xbim2dLine(Geom2d_Line* pLine) : XbimHandle(pLine), parametricUnit(1.0), is3d(true) {};
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return parametricUnit; }; };
				virtual property bool Is3d {bool get() { return is3d; }};
				//Get a Point at the parameter length from the origin
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);
			};
		}
	}
}


