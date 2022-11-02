#pragma once

#include "XCurve2d.h"
#include "../BRep/XAxisPlacement2d.h"
#include<Geom2d_Circle.hxx>
#include <Geom2d_AxisPlacement.hxx>

#define OccCircle2d() Handle(Geom2d_Circle)::DownCast(this->Ref())



namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCircle2d : XCurve2d, public IXCircle
			{
			public:
				XCircle2d(Handle(Geom2d_Circle) hCircle) :XCurve2d(hCircle) { };
				virtual property XCurveType CurveType {XCurveType get() override { return XCurveType::IfcCircle; }; };
				
				virtual property double Radius {double get() { return OccCircle2d()->Radius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get()
				{
					return gcnew XAxisPlacement2d(new Geom2d_AxisPlacement(OccCircle2d()->Position().Location(), OccCircle2d()->Position().XDirection()));
				}};
				
			};
		}
	}
}

