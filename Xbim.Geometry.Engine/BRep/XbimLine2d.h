#pragma once
#include "../BRep/OccExtensions/Geom2d_LineWithMagnitude.h"	
#include "../XbimHandle.h"
#include <Geom2dAdaptor_Curve.hxx>
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

			public ref class XbimLine2d : XbimHandle<Handle(Geom2d_LineWithMagnitude)>, IXLine
			{
					
			public:
				
				XbimLine2d(Handle(Geom2d_LineWithMagnitude) hLine) : XbimHandle(new Handle(Geom2d_LineWithMagnitude)(hLine)) {};
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcLine; }; };
				virtual property IXPoint^ Origin {IXPoint^ get(); };
				virtual property IXVector^ Direction {IXVector^ get(); };
				virtual property double ParametricUnit {double get() { return OccHandle()->Magnitude(); }; };
				virtual property bool Is3d {bool get() { return false; }};
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
				virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);
				virtual property double Length { double get() { return GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(OccHandle())); } }
			};
		}
	}
}


