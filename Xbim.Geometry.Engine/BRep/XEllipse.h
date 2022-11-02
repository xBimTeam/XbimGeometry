#pragma once
#include "XCurve.h"
#include "../XbimHandle.h"
#include "../BRep/XAxis2Placement3d.h"
#include<Geom_Ellipse.hxx>
#include <Geom_Axis2Placement.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
using namespace Xbim::Geometry::Abstractions;
#define OccEllipse() Handle(Geom_Ellipse)::DownCast(this->Ref())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XEllipse : XCurve, IXEllipse
			{
			public:
				XEllipse(Handle(Geom_Ellipse) hEllipse) : XCurve(hEllipse) { };
				virtual property XCurveType CurveType {XCurveType get() override{ return XCurveType::IfcEllipse; }; };								
				virtual property double MajorRadius {double get() { return OccEllipse()->MajorRadius(); }};
				virtual property double MinorRadius {double get() { return OccEllipse()->MinorRadius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get() { return gcnew XAxis2Placement3d(new Geom_Axis2Placement(OccEllipse()->Position())); }};
			};
		}
	}
}

