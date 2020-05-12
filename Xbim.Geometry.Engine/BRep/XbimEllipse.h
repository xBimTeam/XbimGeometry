#pragma once
#include "../XbimHandle.h"
#include "../BRep/XbimAxis2Placement3d.h"
#include<Geom_Ellipse.hxx>
#include <Geom_Axis2Placement.hxx>
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimEllipse : XbimHandle<Handle(Geom_Ellipse)>, IXEllipse
			{
			public:
				XbimEllipse(Handle(Geom_Ellipse) hEllipse) : XbimHandle(new Handle(Geom_Ellipse)(hEllipse)) { };
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcEllipse; }; };
				virtual property bool Is3d {bool get() { return true; }};
				virtual property double MajorRadius {double get() { return OccHandle()->MajorRadius(); }};
				virtual property double MinorRadius {double get() { return OccHandle()->MinorRadius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get() { return gcnew XbimAxis2Placement3d(new Geom_Axis2Placement(OccHandle()->Position())); }};

			};
		}
	}
}

