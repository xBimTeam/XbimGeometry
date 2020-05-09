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
			

			public ref class XbimLine : XbimHandle<Geom_Line>, IXLine
			{
			private:
				double parametricUnit;
				bool is3d = true;
			public:				
				XbimLine(Geom_Line* pLine, double pUnit) : XbimHandle(pLine), parametricUnit(pUnit), is3d(true) {};
				XbimLine(Geom_Line* pLine) : XbimHandle(pLine), parametricUnit(1.0), is3d(true) {};
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return parametricUnit; };};
				virtual property bool Is3d {bool get() {return is3d; }};
				//Get a Point at the parameter length from the origin
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^ %normal);
			};
		}
	}
}


