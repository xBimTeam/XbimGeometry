#include "XbimAxisAlignedBox.h"
#include "XbimPoint.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXPoint^ XbimAxisAlignedBox::CornerMin::get()
			{
				return gcnew XbimPoint(OccHandle().CornerMin());
			}

			IXPoint^ XbimAxisAlignedBox::CornerMax::get()
			{
				return gcnew XbimPoint(OccHandle().CornerMax());
			}
			String^ XbimAxisAlignedBox::Json::get()
			{
				std::ostringstream oss;
				OccHandle().DumpJson(oss);
				return gcnew String(oss.str().c_str());				
			}
			double XbimAxisAlignedBox::LenX::get()
			{
				return CornerMax->X - CornerMin->X;
			}
			double XbimAxisAlignedBox::LenY::get()
			{
				return CornerMax->Y - CornerMin->Y;
			}
			double XbimAxisAlignedBox::LenZ::get()
			{
				return CornerMax->Z - CornerMin->Z;
			}
			double XbimAxisAlignedBox::Gap::get()
			{
				return OccHandle().GetGap();
			}
			
		}
	}
}
