#pragma once
#include "XCurve2d.h"
#include "../BRep/XAxisPlacement2d.h"
#include<Geom2d_Ellipse.hxx>
#include <Geom2d_AxisPlacement.hxx>

#define OccEllipse2d() Handle(Geom2d_Ellipse)::DownCast(this->Ref())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XEllipse2d : XCurve2d, IXEllipse
			{
			public:
				XEllipse2d(Handle(Geom2d_Ellipse) hEllipse) : XCurve2d(hEllipse, XCurveType::IfcEllipse) { };
				
				virtual property double MajorRadius {double get() { return OccEllipse2d()->MajorRadius(); }};
				virtual property double MinorRadius {double get() { return OccEllipse2d()->MinorRadius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get()
				{
					return gcnew XAxisPlacement2d(new Geom2d_AxisPlacement(OccEllipse2d()->Position().Location(), OccEllipse2d()->Position().XDirection()));
				}};
				
			};
		}
	}
}