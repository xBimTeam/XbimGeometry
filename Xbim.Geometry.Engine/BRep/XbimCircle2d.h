#pragma once

#include "../XbimHandle.h"
#include "../BRep/XbimAxisPlacement2d.h"
#include<Geom2d_Circle.hxx>
#include <Geom2d_AxisPlacement.hxx>
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimCircle2d : XbimHandle<Handle(Geom2d_Circle)>, public IXCircle
			{
			public:
				XbimCircle2d(Handle(Geom2d_Circle) hCircle) : XbimHandle(new Handle(Geom2d_Circle)(hCircle)) { };
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcCircle; }; };
				virtual property bool Is3d {bool get() { return false; }};
				virtual property double Radius {double get() { return OccHandle()->Radius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get() 
				{ 
					return gcnew XbimAxisPlacement2d(new Geom2d_AxisPlacement(OccHandle()->Position().Location(), OccHandle()->Position().XDirection())); 
				}};
				virtual property double Length { double get() { return GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(OccHandle())); } }
			};
		}
	}
}

