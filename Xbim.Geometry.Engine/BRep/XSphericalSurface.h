#pragma once
#include "../XbimHandle.h"
#include "XAxis2Placement3d.h"
#include <Geom_SphericalSurface.hxx>
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimSphericalSurface : XbimHandle<Handle(Geom_SphericalSurface)>, IXSphericalSurface
			{
			public:
				XbimSphericalSurface(Handle(Geom_SphericalSurface) hSphere) : XbimHandle(new Handle(Geom_SphericalSurface)(hSphere)) {};
				virtual property XSurfaceType SurfaceType {XSurfaceType get() { return XSurfaceType::IfcSphericalSurface; }}
				virtual property double Radius {double get() { return OccHandle()->Radius(); }}
				virtual property IXAxis2Placement3d^ Position {IXAxis2Placement3d^ get()
				{
					return gcnew XAxis2Placement3d(new Geom_Axis2Placement(OccHandle()->Position().Ax2()));
				}};
			};
		}
	}
}

