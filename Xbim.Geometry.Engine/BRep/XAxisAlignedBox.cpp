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
				if (xbimMat == nullptr || xbimMat == nullptr || xbimMat->IsIdentity) return this;
				
				Bnd_Box bBox(gp_Pnt(CornerMin->X, CornerMin->Y, CornerMin->Z),
					gp_Pnt(CornerMax->X, CornerMax->Y, CornerMax->Z));


				// the non-uniform scale part of the transformation
				if (xbimMat->ScaleX != 1 || xbimMat->ScaleY != 1 || xbimMat->ScaleZ != 1) 
				{ 
					gp_Trsf displaceTrsf;
					const gp_Pnt min = bBox.CornerMin();
					const gp_Pnt max = bBox.CornerMax();
					double x = (max.X() - min.X()) / 2;
					double y = (max.Y() - min.Y()) / 2;
					double z = (max.Z() - min.Z()) / 2;
					displaceTrsf.SetTranslation(gp_Vec(-x, -y ,-z));
					bBox = bBox.Transformed(displaceTrsf);
					Bnd_Box rot = bBox.Transformed(displaceTrsf);
					const gp_Pnt min1 = rot.CornerMin();
					const gp_Pnt max1 = rot.CornerMax();
					double x1 = (max1.X() - min1.X()) / 2;
					double y1 = (max1.Y() - min1.Y()) / 2;
					double z1 = (max1.Z() - min1.Z()) / 2;

					const gp_Pnt minDisplaced = bBox.CornerMin();
					const gp_Pnt maxDisplaced = bBox.CornerMax();
					const gp_Pnt minScaled = gp_Pnt
					   (minDisplaced.X() * xbimMat->ScaleX,
						minDisplaced.Y() * xbimMat->ScaleY,
						minDisplaced.Z() * xbimMat->ScaleZ);

					const gp_Pnt maxScaled = gp_Pnt
					   (maxDisplaced.X() * xbimMat->ScaleX,
						maxDisplaced.Y() * xbimMat->ScaleY,
						maxDisplaced.Z() * xbimMat->ScaleZ);


					displaceTrsf.SetTranslationPart(gp_Vec(x, y, z));
					bBox = Bnd_Box(minScaled, maxScaled);
					bBox = bBox.Transformed(displaceTrsf);
				}

				// the uniform part of the transformation
				gp_Trsf transform = xbimMat->Transform();
				if (transform.Form() != gp_Identity)
				{
					bBox = bBox.Transformed(transform);
				}

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
