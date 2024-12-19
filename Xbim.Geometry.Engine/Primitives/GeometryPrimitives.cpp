#include "GeometryPrimitives.h"
#include "../BRep/XLocation.h"
#include "../BRep/XMatrix.h"
#include "../BRep/XDirection.h"
#include "../BRep/XAxisAlignedBox.h"

using namespace Xbim::Geometry::BRep;

namespace Xbim
{
	namespace Geometry
	{
		namespace Primitives
		{
			IXDirection^ GeometryPrimitives::BuildDirection3d(double x, double y, double z)
			{
				return gcnew Xbim::Geometry::BRep::XDirection(x, y, z);
			};
			IXDirection^ GeometryPrimitives::BuildDirection2d(double x, double y)
			{
				return gcnew Xbim::Geometry::BRep::XDirection(x, y);
			};

			IXPoint^ GeometryPrimitives::BuildPoint3d(double x, double y, double z)
			{
				return gcnew Xbim::Geometry::BRep::XPoint(x, y, z);
			};
			IXPoint^ GeometryPrimitives::BuildPoint2d(double x, double y)
			{
				return gcnew Xbim::Geometry::BRep::XPoint(x, y);
			};

			IXLocation^ GeometryPrimitives::BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz)
			{
				return gcnew Xbim::Geometry::BRep::XLocation(tx, ty, tz, sc, qw, qx, qy, qz);
			}
			IXMatrix^ GeometryPrimitives::BuildMatrix(array<double>^ values)
			{
				if (values == nullptr)
				{
					throw gcnew System::ArgumentNullException("values");
				}
				Graphic3d_Mat4d mat4d;
				int i = 0;
				for (int r = 0; r < 4 && i < 16; r++)
				{
					for (int c = 0; c < 4; c++)
					{
						mat4d.SetValue(r, c, values[i++]);
						if (i >= 16) break;
					}

				}
				return  gcnew XMatrix(mat4d);
			}
			IXAxisAlignedBoundingBox^ GeometryPrimitives::BuildBoundingBox()
			{
				return gcnew XAxisAlignedBox();
			}
			IXAxisAlignedBoundingBox^ GeometryPrimitives::BuildBoundingBox(double x, double y, double z, double sizeX, double sizeY, double sizeZ)
			{
				return gcnew XAxisAlignedBox(x,  y,  z,  sizeX,  sizeY,  sizeZ);
			}
			IXAxisAlignedBoundingBox^ GeometryPrimitives::Moved(IXAxisAlignedBoundingBox^ box, IXLocation^ newLocation)
			{
				if (box->IsVoid) return box;
				auto xbimLoc = dynamic_cast<XLocation^>(newLocation);
				if (newLocation == nullptr || xbimLoc == nullptr || xbimLoc->IsIdentity) return box;
				Bnd_Box bBox(gp_Pnt(box->CornerMin->X, box->CornerMin->Y, box->CornerMin->Z),
					gp_Pnt(box->CornerMax->X, box->CornerMax->Y, box->CornerMax->Z));
				Bnd_Box movedBox = bBox.Transformed(xbimLoc->Ref());
				return gcnew XAxisAlignedBox(movedBox);
			}
		}
	}
}