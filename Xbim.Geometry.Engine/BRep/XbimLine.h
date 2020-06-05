#pragma once
#include "../BRep/OccExtensions/Geom_LineWithMagnitude.h"		
#include "../XbimHandle.h"
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
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
					
			public:				
				XbimLine(Handle(Geom_Line) hLine) : XbimHandle(new Handle(Geom_Line)(hLine)) {};
				virtual property XCurveType CurveType {XCurveType get() {return XCurveType::IfcLine;}; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get(); };
				virtual property bool Is3d {bool get() {return true; }};
				virtual property double FirstParameter {double get()
				{
					return OccHandle()->FirstParameter();
				}};

				virtual property double LastParameter {double get()
				{
					return OccHandle()->LastParameter();
				}};
				//Get a Point at the parameter length from the origin
				virtual IXPoint^ GetPoint(double uParam);
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^ %normal);
				virtual property double Length { double get() { return GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(OccHandle())); } }
			};
		}
	}
}


