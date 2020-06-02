#pragma once
#include "../XbimHandle.h"
#include "../BRep/XbimAxisPlacement2d.h"
#include<Geom2d_Ellipse.hxx>
#include <Geom2d_AxisPlacement.hxx>
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimEllipse2d : XbimHandle<Handle(Geom2d_Ellipse)>, IXEllipse
			{
			public:
				XbimEllipse2d(Handle(Geom2d_Ellipse) hEllipse) : XbimHandle(new Handle(Geom2d_Ellipse)(hEllipse)) { };
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcEllipse; }; };
				virtual property bool Is3d {bool get() { return false; }};
				virtual property double MajorRadius {double get() { return OccHandle()->MajorRadius(); }};
				virtual property double MinorRadius {double get() { return OccHandle()->MinorRadius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get()
				{
					return gcnew XbimAxisPlacement2d(new Geom2d_AxisPlacement(OccHandle()->Position().Location(), OccHandle()->Position().XDirection()));
				}};
				virtual property double Length { double get() { return GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(OccHandle())); } }
			};
		}
	}
}