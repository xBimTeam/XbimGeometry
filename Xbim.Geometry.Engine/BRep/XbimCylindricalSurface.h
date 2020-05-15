#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
#include <Geom_CylindricalSurface.hxx>
#include "../BRep/XbimAxis2Placement3d.h"
using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimCylindricalSurface : XbimHandle<Handle(Geom_CylindricalSurface)>, IXCylindricalSurface
			{
			public:
				XbimCylindricalSurface(Handle(Geom_CylindricalSurface) hCylinder) : XbimHandle(new Handle(Geom_CylindricalSurface)(hCylinder)) {};
				virtual property XSurfaceType SurfaceType {XSurfaceType get() { return XSurfaceType::IfcCylindricalSurface; }}
				virtual property double Radius {double get() { return OccHandle()->Radius(); }}
				virtual property IXAxis2Placement3d^ Position {IXAxis2Placement3d^ get() 
					{ return gcnew XbimAxis2Placement3d(new Geom_Axis2Placement(OccHandle()->Position().Ax2())); }};
			};
		}
	}
}
