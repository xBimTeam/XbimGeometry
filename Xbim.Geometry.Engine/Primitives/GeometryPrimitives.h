#pragma once
#include "../Services/MeshFactors.h"
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Services;
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
				 virtual IXMatrix^ BuildMatrix(array<double>^ values);
				 virtual IXAxisAlignedBoundingBox^ BuildBoundingBox();
				 virtual IXAxisAlignedBoundingBox^ BuildBoundingBox(double x, double y, double z, double sizeX, double sizeY, double sizeZ);
				 virtual IXAxisAlignedBoundingBox^ Moved(IXAxisAlignedBoundingBox^ box, IXLocation^ newLocation);
				 virtual IXMeshFactors^ GetMeshFactors(MeshGranularity granularity, double oneMeter, double precision)
				 {
					 return gcnew MeshFactors(granularity, oneMeter, precision);
				 };
			 };

		 }
	}
}
  

