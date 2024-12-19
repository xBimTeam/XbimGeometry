#include "XAxisAlignedBox.h"
#include "XPoint.h"
#include "XLocation.h"
#include "XMatrix.h"
#include "../Extensions//ToGTransform.h"


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

			IXAxisAlignedBoundingBox^ XAxisAlignedBox::Transformed(IXLocation^ location)
			{
				if (IsVoid) return this;
				auto xbimLoc = dynamic_cast<XLocation^>(location);
				if (location == nullptr || xbimLoc == nullptr || xbimLoc->IsIdentity) return this;
				Bnd_Box bBox(gp_Pnt(CornerMin->X, CornerMin->Y, CornerMin->Z),
					gp_Pnt(CornerMax->X, CornerMax->Y, CornerMax->Z));
				Bnd_Box movedBox = bBox.Transformed(xbimLoc->Ref());
				return gcnew XAxisAlignedBox(movedBox);
			}

			IXAxisAlignedBoundingBox^ XAxisAlignedBox::Transformed(IXMatrix^ transformation)
			{
				if (IsVoid) return this;
				auto xbimMat = dynamic_cast<XMatrix^>(transformation);
				if (xbimMat == nullptr || xbimMat->IsIdentity) return this;
				Bnd_Box bBox(gp_Pnt(CornerMin->X, CornerMin->Y, CornerMin->Z),
					gp_Pnt(CornerMax->X, CornerMax->Y, CornerMax->Z));
				gp_Trsf transform = xbimMat->Transform();
				Bnd_Box movedBox = bBox.Transformed(transform);
				return gcnew XAxisAlignedBox(bBox);
			}
			
			IXAxisAlignedBoundingBox^ XAxisAlignedBox::Translated(double x, double y, double z)
			{
				gp_Vec translation(x, y, z);
				Bnd_Box bBox(Ref().CornerMin().Translated(translation), Ref().CornerMax().Translated(translation));
				return gcnew XAxisAlignedBox(bBox);		
			}

			IXAxisAlignedBoundingBox^ XAxisAlignedBox::Translated(IXPoint^ translation)
			{
				return Translated(translation->X, translation->Y, translation->Z);
			}

			IXPoint^ XAxisAlignedBox::Centroid::get()
			{
				auto vCentroid = gp_Vec(Ref().CornerMin(), Ref().CornerMax()) / 2;
				auto centroid = Ref().CornerMin().Translated(vCentroid);
				return gcnew XPoint(centroid);
			}
		}
	}
}
