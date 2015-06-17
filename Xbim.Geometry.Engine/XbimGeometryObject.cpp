#include "XbimGeometryObject.h"

namespace Xbim
{
	namespace Geometry
	{
		XbimGeometryObject::XbimGeometryObject()
		{
		}

		XbimRect3D XbimGeometryObject::BoundingBox::get()
		{
			return XbimRect3D::Empty;
		}
	}
}
