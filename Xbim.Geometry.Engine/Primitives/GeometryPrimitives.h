#pragma once

using namespace Xbim::Geometry::Abstractions;

 namespace Xbim
{
	 namespace Geometry
	 {
		 namespace Primitives
		 {
			 public ref class GeometryPrimitives : IXGeometryPrimitives
			 {
			 public:
				 virtual IXDirection^ BuildDirection3d(double x, double y, double z);
				 virtual IXDirection^ BuildDirection2d(double x, double y);
				 virtual IXPoint^ BuildPoint3d(double x, double y, double z);
				 virtual IXPoint^ BuildPoint2d(double x, double y);
				 virtual IXLocation^ BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz);
			 };

		 }
	}
}
  

