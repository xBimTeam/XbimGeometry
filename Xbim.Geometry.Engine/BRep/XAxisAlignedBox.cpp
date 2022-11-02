#include "XAxisAlignedBox.h"
#include "XPoint.h"
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXPoint^ XAxisAlignedBox::CornerMin::get()
			{
				if (OccHandle().IsVoid())
					return gcnew XPoint(0, 0, 0);
				else
					return gcnew XPoint(OccHandle().CornerMin());
			}

			IXPoint^ XAxisAlignedBox::CornerMax::get()
			{
				if (OccHandle().IsVoid())
					return gcnew XPoint(0, 0, 0);
				else
					return gcnew XPoint(OccHandle().CornerMax());
			}
			String^ XAxisAlignedBox::Json::get()
			{
				std::ostringstream oss;
				OccHandle().DumpJson(oss);
				return gcnew String(oss.str().c_str());
			}
			double XAxisAlignedBox::LenX::get()
			{
				return CornerMax->X - CornerMin->X;
			}
			double XAxisAlignedBox::LenY::get()
			{
				return CornerMax->Y - CornerMin->Y;
			}
			double XAxisAlignedBox::LenZ::get()
			{
				return CornerMax->Z - CornerMin->Z;
			}
			double XAxisAlignedBox::Gap::get()
			{
				return OccHandle().GetGap();
			}
			bool XAxisAlignedBox::IsVoid::get()
			{
				return OccHandle().IsVoid();
			}

			IXAxisAlignedBoundingBox^ XAxisAlignedBox::Union(IXAxisAlignedBoundingBox^ other)
			{
				if (other->IsVoid) return this;
				if (this->IsVoid) return other;
				Bnd_Box otherBbox(gp_Pnt(other->CornerMin->X, other->CornerMin->Y, other->CornerMin->Z),
					gp_Pnt(other->CornerMax->X, other->CornerMax->Y, other->CornerMax->Z));
				otherBbox.Add(Ref());
				return gcnew XAxisAlignedBox(otherBbox);
			}

		}
	}
}
