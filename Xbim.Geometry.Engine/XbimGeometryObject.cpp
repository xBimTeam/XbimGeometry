#include "XbimGeometryObject.h"
#include "XbimOccShape.h"
#include <BRepTools.hxx>
namespace Xbim
{
	namespace Geometry
	{
		

		XbimRect3D XbimGeometryObject::BoundingBox::get()
		{
			
			return XbimRect3D::Empty;
		}

		String^ XbimGeometryObject::ToBRep::get()
		{
			std::ostringstream oss;
			if (dynamic_cast<XbimOccShape^>(this))
			{
				BRepTools::Write((XbimOccShape^)this,oss);
				return gcnew String(oss.str().c_str());
			}
			//otherwise we don't do it
			return String::Empty;
		}
	}
}
