#pragma once
#include "XbimGeometryObject.h"
#include <TopLoc_Location.hxx>
#include <gp_GTrsf.hxx> 
#include <gp_Trsf.hxx> 
#include <gp_Pln.hxx> 
 
using namespace Xbim::Ifc2x3::GeometryResource;
using namespace Xbim::Ifc2x3::GeometricConstraintResource;
using namespace Xbim::Ifc2x3::IO;
using namespace Xbim::Common::Exceptions;
using namespace Xbim::Common::Geometry;
namespace Xbim
{
	namespace Geometry
	{

		ref class XbimGeomPrim
		{
		private:

		public:
			XbimGeomPrim(void);
			// Converts a Local Placement into a TopLoc_Location
			static TopLoc_Location ToLocation(IfcObjectPlacement^ placement);
			// Converts a Placement into a TopLoc_Location
			static TopLoc_Location ToLocation(IfcPlacement^ placement);
			// Converts a IfcAxis2Placement into a TopLoc_Location
			static TopLoc_Location ToLocation(IfcAxis2Placement^ placement);
			// Converts an Axis2Placement3D into a TopLoc_Location
			static TopLoc_Location ToLocation(IfcAxis2Placement3D^ axis3D);
			// Converts an Axis2Placement3D into a gp_Ax3
			static gp_Ax3 ToAx3(IfcAxis2Placement3D^ axis3D);
			// Converts an Axis2Placement2D into a TopLoc_Location
			static TopLoc_Location ToLocation(IfcAxis2Placement2D^ axis2D);
			// Converts an CartesianTransformationOperator into a gp_GTrsf
			static gp_Trsf ToTransform(IfcCartesianTransformationOperator^ tForm);
			// Converts an CartesianTransformationOperator3D into a gp_GTrsf
			static gp_Trsf ToTransform(IfcCartesianTransformationOperator3D^ ct3D);
			// Converts an CartesianTransformationOperator3DnonUniform into a gp_GTrsf
			static gp_GTrsf ToTransform(IfcCartesianTransformationOperator3DnonUniform^ ct3D);
			// Converts an CartesianTransformationOperator2D into a gp_GTrsf
			static gp_Trsf ToTransform(IfcCartesianTransformationOperator2D^ ct2D);
			// Converts an Matrix3D into a gp_GTrsf
			static gp_Trsf ToTransform(XbimMatrix3D m3D);
			// Converts an Axis2Placement3D to a Plane
			static gp_Pln ToPlane(IfcAxis2Placement3D^ axis3D);
			//converts an Axis2Placement2D into a Transform matrix
			static gp_Trsf ToTransform(IfcAxis2Placement3D^ axis3D);
			static XbimMatrix3D ToMatrix3D(const TopLoc_Location& location);
			// Builds a windows Matrix3D from a CartesianTransformationOperator3D
			static XbimMatrix3D ConvertMatrix3D(IfcCartesianTransformationOperator3D ^ stepTransform);
			static XbimMatrix3D ConvertMatrix3D(IfcObjectPlacement ^ placement);
		};

	}
}