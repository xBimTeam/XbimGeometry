#include "XbimGeometryObject.h"
#include "XbimOccShape.h"
#include <BRepTools.hxx>
namespace Xbim
{
	namespace Geometry
	{
				

		String^ XbimGeometryObject::ToBRep::get()
		{
			if (!IsValid)
				return String::Empty;
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
