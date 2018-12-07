#pragma once
#include "XbimGeometryObject.h"
#include <TopLoc_Location.hxx>
#include <gp_GTrsf.hxx> 
#include <gp_Trsf.hxx> 
#include <gp_Pln.hxx> 
 
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Common::Exceptions;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::MeasureResource;

 
namespace Xbim
{
	namespace Geometry
	{


	    public ref class XbimConvert
		{
		private:
			static Ifc4::GeometryResource::IfcDimensionCount dimensions3D = Ifc4::GeometryResource::IfcDimensionCount(3);
		public:
			XbimConvert(void);
			// Converts a Local Placement into a TopLoc_Location
			static TopLoc_Location ToLocation(IIfcObjectPlacement^ placement, ILogger^ logger);
			// Converts a Placement into a TopLoc_Location
			static TopLoc_Location ToLocation(IIfcPlacement^ placement);
			// Converts a IfcAxis2Placement into a TopLoc_Location
			static TopLoc_Location ToLocation(IIfcAxis2Placement^ placement);
			// Converts an Axis2Placement3D into a TopLoc_Location
			static TopLoc_Location ToLocation(IIfcAxis2Placement3D^ axis3D);
			// Converts an Axis2Placement3D into a gp_Ax3
			static gp_Ax3 ToAx3(IIfcAxis2Placement3D^ axis3D);
			// Converts an Axis2Placement2D into a TopLoc_Location
			static TopLoc_Location ToLocation(IIfcAxis2Placement2D^ axis2D);
			// Converts an CartesianTransformationOperator into a gp_GTrsf
			static gp_Trsf ToTransform(IIfcCartesianTransformationOperator^ tForm);
			// Converts an CartesianTransformationOperator3D into a gp_GTrsf
			static gp_Trsf ToTransform(IIfcCartesianTransformationOperator3D^ ct3D);
			// Converts an CartesianTransformationOperator3DnonUniform into a gp_GTrsf
			static gp_GTrsf ToTransform(IIfcCartesianTransformationOperator3DnonUniform^ ct3D);
			// Converts an CartesianTransformationOperator2D into a gp_GTrsf
			static gp_Trsf ToTransform(IIfcCartesianTransformationOperator2D^ ct2D);
			// Converts an Matrix3D into a gp_GTrsf
			static gp_Trsf ToTransform(XbimMatrix3D m3D);
			// Converts an Axis2Placement3D to a Plane
			static gp_Pln ToPlane(IIfcAxis2Placement3D^ axis3D);
			//converts an Axis2Placement2D into a Transform matrix
			static gp_Trsf ToTransform(IIfcAxis2Placement3D^ axis3D);
			static XbimMatrix3D ToMatrix3D(const TopLoc_Location& location);
			static XbimMatrix3D ToMatrix3D(IIfcAxis2Placement3D^ axis3);
			// Builds a windows Matrix3D from a CartesianTransformationOperator3D
			static XbimMatrix3D ConvertMatrix3D(IIfcCartesianTransformationOperator3D ^ stepTransform);
			static XbimMatrix3D ConvertMatrix3D(IIfcObjectPlacement ^ placement, ILogger^ logger);
			static bool IsEqual(IIfcCartesianPoint^ ptA, IIfcCartesianPoint^ ptB, double tolerance);
			static double DistanceSquared(IIfcCartesianPoint^ pt1, IIfcCartesianPoint^ pt2);
			static bool Is3D(IIfcPolyline^ pline);
			static bool Is3D(IIfcPolyLoop^ pLoop);
			static bool IsPolygon(IIfcPolyLoop^ pLoop);
			static double GetZValueOrZero(IIfcCartesianPoint^ point);
			static double GetZValueOrZero(IIfcDirection^ dir);
			static double GetZValueOrZero(IIfcVector^ vec);
			static XbimVector3D NewellsNormal(IIfcPolyLoop^ loop);
			static gp_Pnt GetPoint3d(IIfcCartesianPoint^ cartesian);
			static gp_Pnt2d GetPoint2d(IIfcCartesianPoint^ cartesian);
			static gp_Dir GetDir3d(IIfcDirection^ dir);
			static gp_Dir2d GetDir2d(IIfcDirection^ dir);
			static gp_Vec2d GetDir2d(IEnumerable<IfcLengthMeasure>^ offsets);
			static gp_Vec GetDir3d(IEnumerable<IfcLengthMeasure>^ offsets);
			static gp_Pnt GetPoint3d(IIfcAxis2Placement^ placement);
			static gp_Vec GetRefDir3d(IIfcAxis2Placement^ placement);
			static gp_Vec GetAxisDir3d(IIfcAxis2Placement^ placement);
			static gp_Ax3 ToAx3(IIfcAxis2Placement2D^ axis2D);
			static gp_Ax3 ToAx3(IIfcAxis2Placement^ axis2);
		};

	}
}