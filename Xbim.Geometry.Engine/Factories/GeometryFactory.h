//Geometric Processor Factory, builds  entities used for algebraic calculation etc from their Ifc counterparts
#pragma once
#include "Unmanaged/NGeometryFactory.h"
#include "FactoryBase.h"


#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Ax22d.hxx>

#include <TColgp_SequenceOfPnt2d.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TopLoc_Location.hxx>

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			
			public ref class GeometryFactory : public FactoryBase<NGeometryFactory>, IXGeometryFactory
			{

			public:

				GeometryFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NGeometryFactory()) {}

				//builds a 3d point, if the Ifc point is 2d the Z coordinate is 0
				gp_Pnt BuildPoint3d(IIfcCartesianPoint^ ifcPoint);
				gp_Pnt BuildPoint3d(IXPoint^ xPoint);

				//builds a 2d point, if the Ifc point is 3d an exception is thrown
				bool BuildPoint2d(IIfcCartesianPoint^ ifcPoint, gp_Pnt2d& pnt2d);
				gp_Pnt2d BuildPoint2d(IIfcCartesianPoint^ ifcPoint);
				
				bool BuildDirection3d(IIfcDirection^ ifcDir, gp_Vec& pnt);

				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				//throws a XbimGeometryFactoryException if the normal of the vector is 0
				bool BuildDirection2d(IIfcDirection^ ifcDir, gp_Vec2d& dir);

				//builds a 3d vector, if the Ifc Vector is 2d the Z component is 0
				//throws a XbimGeometryFactoryException if the normal of the vector is 0
				bool BuildVector3d(IIfcVector^ ifcVec, gp_Vec& vec);

				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				bool BuildVector2d(IIfcVector^ ifcVec, gp_Vec2d& vec);

				bool BuildAxis2Placement3d(IIfcAxis2Placement3D^ axis2, gp_Ax2& ax2);
				bool BuildAxis2Placement2d(IIfcAxis2Placement2D^ axis, gp_Ax22d& ax22d);

				bool ToLocation(IIfcAxis2Placement2D^ axis2D, TopLoc_Location& loc);
				gp_Trsf ToTransform(Xbim::Common::Geometry::XbimMatrix3D m3D);
				

				void GetPolylinePoints3d(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt& points);
				void GetPolylinePoints2d(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt2d& points);


				virtual bool IsFacingAwayFrom(IXFace^ face, IXDirection^ direction);
				virtual IXAxis2Placement2d^ GetAxis2Placement2d(IXPoint^ location, IXVector^ XaxisDirection);
				virtual IXDirection^ BuildDirection3d(double x, double y, double z);
				virtual IXDirection^ BuildDirection2d(double x, double y);
				virtual IXPoint^ BuildPoint3d(double x, double y, double z);
				virtual IXPoint^ BuildPoint2d(double x, double y);
				virtual IXPlane^ BuildPlane(IIfcPlane^ plane);
				virtual double Distance(IXPoint^ a, IXPoint^ b);
				virtual double IsEqual(IXPoint^ a, IXPoint^ b, double tolerance);
				virtual IXDirection^ NormalAt(IXFace^ face, IXPoint^ position, double tolerance);

				virtual IXLocation^ BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz);

				static double GetDeterminant(double x1, double y1, double x2, double y2);
				static double Area(const TColgp_SequenceOfPnt2d& points2d);
			};
		}
	}
}
