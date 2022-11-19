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
			public ref class GeometryFactory : FactoryBase<NGeometryFactory>, IXGeometryFactory
			{
			
			public:
				GeometryFactory(Xbim::Geometry::Services::ModelService^ modelService) : FactoryBase(modelService, new NGeometryFactory()) {}
				
				//builds a 3d point, if the Ifc point is 2d the Z coordinate is 0
				gp_Pnt BuildPoint(IIfcCartesianPoint^ ifcPoint);

				//builds a 2d point, if the Ifc point is 3d an exception is thrown
				gp_Pnt2d BuildPoint2d(IIfcCartesianPoint^ ifcPoint);

				//builds a 3d direction, if the Ifc Direction is 2d the Z component is 0
				gp_Dir BuildDirection(IIfcDirection^ ifcDir);

				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				//throws a XbimGeometryFactoryException if the normal of the vector is 0
				gp_Dir2d BuildDirection2d(IIfcDirection^ ifcDir);

				//builds a 3d vector, if the Ifc Vector is 2d the Z component is 0
				//throws a XbimGeometryFactoryException if the normal of the vector is 0
				gp_Vec BuildVector(IIfcVector^ ifcVec);

				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				gp_Vec2d BuildVector2d(IIfcVector^ ifcVec);

				gp_Ax2 BuildAxis2Placement(IIfcAxis2Placement3D^ axis2);
				gp_Ax22d BuildAxis2Placement2d(IIfcAxis2Placement2D^ axis);

				virtual IXAxis2Placement2d^ GetAxis2Placement2d(IXPoint^ location, IXVector^ XaxisDirection);

				void GetPolylinePoints(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt& points);
				void GetPolylinePoints2d(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt2d& points);
				TopLoc_Location ToLocation(IIfcAxis2Placement2D^ axis2D);
				gp_Trsf ToTransform(Xbim::Common::Geometry::XbimMatrix3D m3D);
				virtual bool IsFacingAwayFrom(IXFace^ face, IXDirection^ direction);
				virtual IXDirection^ BuildDirection(double x, double y, double z);
				
				virtual IXPoint^ BuildPoint(double x, double y, double z);
				

				virtual IXPlane^ BuildPlane(IIfcPlane^ plane);
				virtual double Distance(IXPoint^ a, IXPoint^ b);
				virtual double IsEqual(IXPoint^ a, IXPoint^ b, double tolerance);
				virtual IXDirection^ NormalAt(IXFace^ face, IXPoint^ position, double tolerance);
				
				// Inherited via IXGeometryProcedures
				virtual Xbim::Geometry::Abstractions::IXVisualMaterial^ BuildVisualMaterial(System::String^ name, Xbim::Ifc4::Interfaces::IIfcSurfaceStyleElementSelect^ styling);

				virtual Xbim::Geometry::Abstractions::IXVisualMaterial^ BuildVisualMaterial(System::String^ name);
				// Inherited via IXGeometryProcedures
				virtual Xbim::Geometry::Abstractions::IXColourRGB^ BuildColourRGB(double red, double green, double blue);
				virtual Xbim::Geometry::Abstractions::IXShapeColour^ BuildShapeColour(System::String^ name, IIfcSurfaceStyleElementSelect^ surfaceStyle);
				virtual Xbim::Geometry::Abstractions::IXLocation^ BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz);
			};
		}
	}
}
