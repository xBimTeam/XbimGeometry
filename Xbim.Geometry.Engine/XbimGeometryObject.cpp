
#include <BRepTools.hxx>
#include "XbimGeometryObject.h"
#include "XbimOccShape.h"
namespace Xbim
{
	namespace Geometry
	{
				

		System::String^ XbimGeometryObject::ToBRep::get()
		{
			if (!IsValid)
				return System::String::Empty;
			std::ostringstream oss;
			if (dynamic_cast<XbimOccShape^>(this))
			{
				BRepTools::Write((XbimOccShape^)this,oss);
				return gcnew System::String(oss.str().c_str());
			}
		

			//otherwise we don't do it
			return System::String::Empty;
		}
	}
}
