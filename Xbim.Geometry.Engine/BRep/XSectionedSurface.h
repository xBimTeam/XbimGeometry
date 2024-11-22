#pragma once
#include "XShape.h"
#include "XCurve.h"
#include <Geom_SurfaceOfLinearExtrusion.hxx>

using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XSectionedSurface : XShape, IXSectionedSurface
			{
				private:
					IXCurve^ _directrix;
				public:
					XSectionedSurface(const TopoDS_Shape& surface) : XShape(new TopoDS_Shape(surface))
					{
					}
					virtual property IXCurve^ Directrix {IXCurve^ get() { return _directrix; }; void set(IXCurve^ directrix) { _directrix = directrix; }}
					virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Shell; }}
					virtual property bool IsUPeriodic {bool get() { return false; }}
					virtual property bool IsVPeriodic {bool get() { return false; }}
					virtual property XSurfaceType SurfaceType {virtual XSurfaceType get() { return XSurfaceType::IfcSectionedSurface; }}
			};
		}
	}
}
