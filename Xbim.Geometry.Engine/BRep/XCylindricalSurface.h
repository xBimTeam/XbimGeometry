#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include <Geom_CylindricalSurface.hxx>
#include "../BRep/XAxis2Placement3d.h"
using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCylindricalSurface : public XSurface, IXCylindricalSurface
			{
			public:
				XCylindricalSurface(Handle(Geom_CylindricalSurface) hCylinder) : XSurface(hCylinder) {};
				virtual property XSurfaceType SurfaceType {XSurfaceType get() override { return XSurfaceType::IfcCylindricalSurface; }}
				virtual property double Radius {double get() { return Handle(Geom_CylindricalSurface)::DownCast(Ref())->Radius(); }}
				virtual property IXAxis2Placement3d^ Position {IXAxis2Placement3d^ get() 
					{ return gcnew XAxis2Placement3d(new Geom_Axis2Placement(Handle(Geom_CylindricalSurface)::DownCast(Ref())->Position().Ax2())); }};
			};
		}
	}
}
