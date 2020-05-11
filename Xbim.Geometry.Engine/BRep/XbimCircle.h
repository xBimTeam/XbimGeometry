#pragma once
#include "../XbimHandle.h"
#include "../BRep/XbimAxis2Placement.h"
#include<Geom_Circle.hxx>
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimCircle :XbimHandle<Handle(Geom_Circle)>, IXCircle
			{
			public:
				XbimCircle(Handle(Geom_Circle) hCircle) : XbimHandle(new Handle(Geom_Circle)(hCircle)) { };
				virtual property XCurveType CurveType {XCurveType get() { return XCurveType::IfcCircle; }; };
				virtual property bool Is3d {bool get() { return true; }};
				virtual property double Radius {double get() { return OccHandle()->Radius(); }};
				virtual property IXAxis2Placement^ Position {IXAxis2Placement^ get() { return gcnew XbimAxis2Placement(new Geom_Axis2Placement(OccHandle()->Position())); }};


				////Get a Point at the parameter length from the origin
				//virtual IXPoint^ GetPoint(double uParam);
				//virtual IXPoint^ GetFirstDerivative(double uParam, [Out] IXVector^% normal);
			};
		}
	}
}

