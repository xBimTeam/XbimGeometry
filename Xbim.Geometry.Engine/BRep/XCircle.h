#pragma once
#include "XCurve.h"
#include "../XbimHandle.h"
#include "../BRep/XAxis2Placement3d.h"
#include<Geom_Circle.hxx>
#include <Geom_Axis2Placement.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
using namespace Xbim::Geometry::Abstractions;
#define OccCircle() Handle(Geom_Circle)::DownCast(this->Ref())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCircle :XCurve, IXCircle
			{
			public:
				XCircle(Handle(Geom_Circle) hCircle) : XCurve(hCircle, XCurveType::IfcCircle) { };
				
				virtual property double Radius {double get() { return OccCircle()->Radius(); }};
				virtual property IXAxisPlacement^ Position {IXAxisPlacement^ get() { return gcnew XAxis2Placement3d(new Geom_Axis2Placement(OccCircle()->Position())); }};
				

			};
		}
	}
}

