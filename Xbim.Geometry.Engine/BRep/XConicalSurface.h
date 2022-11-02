#pragma once
#include "../XbimHandle.h"
#include "../BRep/XAxis2Placement3d.h"
#include <Geom_ConicalSurface.hxx>
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			ref class XConicalSurface : XbimHandle<Handle(Geom_ConicalSurface)>, IXConicalSurface
			{
			public:
				XConicalSurface(Handle(Geom_ConicalSurface) hConic) : XbimHandle(new Handle(Geom_ConicalSurface)(hConic)) { };
				virtual property XSurfaceType SurfaceType {XSurfaceType get() { return XSurfaceType::IfcSurfaceOfRevolution; }}
				virtual property double Radius {double get() { return OccHandle()->RefRadius(); }}
				virtual property IXAxis2Placement3d^ Position {IXAxis2Placement3d^ get() { return gcnew XAxis2Placement3d(new Geom_Axis2Placement( OccHandle()->Position().Ax2())); }};
			};
		}
	}
}

