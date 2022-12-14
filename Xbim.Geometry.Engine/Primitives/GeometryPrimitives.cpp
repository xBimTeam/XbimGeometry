#include "GeometryPrimitives.h"
#include "../BRep/XLocation.h"

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
        }
    }
}