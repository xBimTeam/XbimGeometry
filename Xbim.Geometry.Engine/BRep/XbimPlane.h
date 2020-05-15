#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include <Geom_Plane.hxx>
using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimPlane : XbimHandle<Handle(Geom_Plane)>, IXPlane
			{
			public:
				XbimPlane(const Handle(Geom_Plane) hPlane) : XbimHandle(new Handle(Geom_Plane)(hPlane)) {};
				virtual property XSurfaceType SurfaceType {XSurfaceType get() { return XSurfaceType::IfcPlane; }}
				virtual property IXPoint^ Location {IXPoint^ get(); };
				virtual property IXDirection^ Axis {IXDirection^ get(); };
				virtual property IXDirection^ RefDirection {IXDirection^ get(); };
			};
		}
	}
}
