#pragma once
#include "XSurface.h"

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
			public ref class XPlane : XSurface, IXPlane
			{
			public:
				XPlane(const Handle(Geom_Plane) hPlane) : XSurface(hPlane) {};
				property XSurfaceType SurfaceType {virtual XSurfaceType get() override { return XSurfaceType::IfcPlane; }}
				property IXPoint^ Location {virtual IXPoint^ get(); };
				property IXDirection^ Axis {virtual IXDirection^ get(); };
				property IXDirection^ RefDirection {virtual IXDirection^ get(); };
			};
		}
	}
}
