#pragma once
#include "XSurface.h"
#include "XAxis2Placement3d.h"
#include <Geom_SphericalSurface.hxx>
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XSphericalSurface : XSurface, IXSphericalSurface
			{
			public:
				XSphericalSurface(Handle(Geom_SphericalSurface) hSphere) : XSurface(hSphere){};
				virtual property XSurfaceType SurfaceType {XSurfaceType get() override { return XSurfaceType::IfcSphericalSurface; }}
				virtual property double Radius {double get() { return Handle(Geom_SphericalSurface)::DownCast(Ref())->Radius(); }}
				virtual property IXAxis2Placement3d^ Position {IXAxis2Placement3d^ get()
				{
					return gcnew XAxis2Placement3d(new Geom_Axis2Placement(Handle(Geom_SphericalSurface)::DownCast(Ref())->Position().Ax2()));
				}};
			};
		}
	}
}

