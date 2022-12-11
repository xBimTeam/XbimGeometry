#pragma once
#include "XSurface.h"
#include "../BRep/XAxis2Placement3d.h"
#include <Geom_ConicalSurface.hxx>
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			ref class XConicalSurface : XSurface, IXConicalSurface
			{
			public:
				XConicalSurface(Handle(Geom_ConicalSurface) hConic) : XSurface(hConic) { };
				virtual property XSurfaceType SurfaceType {XSurfaceType get() override { throw gcnew System::NotImplementedException("Ifc does not implemented Conical Surfaces"); }}
				virtual property double Radius {double get() { return Handle(Geom_ConicalSurface)::DownCast(Ref())->RefRadius(); }}
				virtual property IXAxis2Placement3d^ Position {IXAxis2Placement3d^ get() { return gcnew XAxis2Placement3d(new Geom_Axis2Placement(Handle(Geom_ConicalSurface)::DownCast(Ref())->Position().Ax2())); }};
			};
		}
	}
}

