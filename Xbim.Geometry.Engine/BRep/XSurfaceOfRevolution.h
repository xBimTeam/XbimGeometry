#pragma once
#include "XSurface.h"
#include "XCurve.h"
#include "XDirection.h"
#include <Geom_SurfaceOfRevolution.hxx>
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			ref class XSurfaceOfRevolution : XSurface, IXSurfaceOfRevolution
			{
			public:
				XSurfaceOfRevolution(Handle(Geom_SurfaceOfRevolution) hRev) : XSurface(hRev) { };
				virtual property XSurfaceType SurfaceType {XSurfaceType get() override { return XSurfaceType::IfcSurfaceOfRevolution; }}
				virtual property IXCurve^ BasisCurve {IXCurve^ get() { return XCurve::GeomToXCurve( Handle(Geom_SurfaceOfRevolution)::DownCast(Ref())->BasisCurve()); }}
				virtual property IXDirection^ Direction { IXDirection^ get() { return gcnew XDirection(Handle(Geom_SurfaceOfRevolution)::DownCast(Ref())->Direction()); }}
			};
		}
	}
}


