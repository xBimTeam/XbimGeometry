#include "GeometryProcedures.h"
#include <gp_Ax2.hxx>
#include <TColgp_SequenceOfPnt2d.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopLoc_Datum3D.hxx>

#include <TopoDS_Shape.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRepGProp_Face.hxx>
#include <Geom_Plane.hxx>
#include <gp_Ax3.hxx>
#include <math.h>



#include "../BRep/XCompound.h"
#include "../BRep/XEdge.h"
#include "../BRep/XSolid.h"
#include "../BRep/XFace.h"
#include "../BRep/XShell.h"
#include "../BRep/XVertex.h"
#include "../BRep/XWire.h"
#include "../BRep/XDirection.h"
#include "../visual//VisualMaterial.h"
#include "../visual//XbimShapeColour.h"

#include "../BRep/XAxisPlacement2d.h"
#include "../BRep/XPlane.h"
#include <ShapeAnalysis_Surface.hxx>
#include <GeomLProp_SLProps.hxx>
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Visual;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXDirection^ GeometryProcedures::BuildDirection(double x, double y, double z)
			{
				return gcnew Xbim::Geometry::BRep::XDirection(x, y, z);
			};
			IXPoint^ GeometryProcedures::BuildPoint(double x, double y, double z)
			{
				return gcnew XPoint(x, y, z);
			};

			gp_Pnt GeometryProcedures::BuildPoint(IIfcCartesianPoint^ ifcPoint)
			{
				return gp_Pnt(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1], (int)ifcPoint->Dim == 3 ? (double)ifcPoint->Coordinates[2] : 0.0);
			}

			gp_Pnt2d GeometryProcedures::BuildPoint2d(IIfcCartesianPoint^ ifcPoint)
			{
				if ((int)ifcPoint->Dim == 3) throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d point from a 3d point. ");
				return gp_Pnt2d(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1]);
			}

			gp_Dir GeometryProcedures::BuildDirection(IIfcDirection^ ifcDir)
			{
				try
				{
					return gp_Dir(ifcDir->DirectionRatios[0], ifcDir->DirectionRatios[1], (int)ifcDir->Dim == 3 ? (double)ifcDir->DirectionRatios[2] : 0.0);
				}
				catch (...) //this will only fail if the normal is zero
				{
					throw gcnew XbimGeometryFactoryException("Input direction has zero normal");
				}
			}

			gp_Dir2d GeometryProcedures::BuildDirection2d(IIfcDirection^ ifcDir)
			{
				try
				{
					if ((int)ifcDir->Dim == 3) throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d direction from a 3d direction. ");
					return gp_Dir2d(ifcDir->DirectionRatios[0], ifcDir->DirectionRatios[1]);
				}
				catch (...) //this will only fail if the normal is zero
				{
					throw gcnew XbimGeometryFactoryException("Input direction has zero normal");
				}
			}
			gp_Vec GeometryProcedures::BuildVector(IIfcVector^ ifcVec)
			{
				try
				{
					gp_Vec vec(ifcVec->Orientation->DirectionRatios[0], ifcVec->Orientation->DirectionRatios[1], (int)ifcVec->Dim == 3 ? (double)ifcVec->Orientation->DirectionRatios[2] : 0.0);
					vec.Multiply(ifcVec->Magnitude);
					return vec;
				}
				catch (...) //this will only fail if the normal is zero
				{
					throw gcnew XbimGeometryFactoryException("Input vector has zero normal");
				}
			}

			gp_Vec2d GeometryProcedures::BuildVector2d(IIfcVector^ ifcVec)
			{
				try
				{
					if ((int)ifcVec->Dim == 3) throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d vector from a 3d vector. ");
					gp_Vec2d vec(ifcVec->Orientation->DirectionRatios[0], ifcVec->Orientation->DirectionRatios[1]);
					vec.Multiply(ifcVec->Magnitude);
					return vec;
				}
				catch (...) //this will only fail if the normal is zero
				{
					throw gcnew XbimGeometryFactoryException("Input vector has zero normal");
				}
			}

			gp_Ax2 GeometryProcedures::BuildAxis2Placement(IIfcAxis2Placement3D^ axis2)
			{
				if (axis2->Axis == nullptr || axis2->RefDirection == nullptr) //both have to be given if one is null use the defaults
					return gp_Ax2(BuildPoint(axis2->Location), gp::DZ(), gp::DX());
				else return gp_Ax2(
					BuildPoint(axis2->Location),
					BuildDirection(axis2->Axis),
					BuildDirection(axis2->RefDirection)
				);
			}

			gp_Ax22d GeometryProcedures::BuildAxis2Placement2d(IIfcAxis2Placement2D^ axis2d)
			{
				if (axis2d->RefDirection == nullptr)
					return gp_Ax22d(BuildPoint2d(axis2d->Location), gp::DX2d());
				else
					return gp_Ax22d(
						BuildPoint2d(axis2d->Location),
						BuildDirection2d(axis2d->RefDirection)
					);
			}
			IXAxis2Placement2d^ GeometryProcedures::GetAxis2Placement2d(IXPoint^ location, IXVector^ XaxisDirection)
			{

				gp_Ax2d axis = gp_Ax2d(
					gp_Pnt2d(location->X, location->Y),
					gp_Dir2d(XaxisDirection->X, XaxisDirection->Y)
				);
				Handle(Geom2d_AxisPlacement) hPlacement = new Geom2d_AxisPlacement(axis);
				return gcnew XAxisPlacement2d(hPlacement);
			}
			void GeometryProcedures::GetPolylinePoints(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt& points)
			{
				int i = 1;
				for each (IIfcCartesianPoint ^ ifcPoint in ifcPolyline->Points)
				{
					gp_Pnt pnt = BuildPoint(ifcPoint);
					points.SetValue(i, pnt);
					i++;
				}
			}
			TopLoc_Location GeometryProcedures::ToLocation(IIfcAxis2Placement2D^ axis2D)
			{
				gp_Pnt2d loc = BuildPoint2d(axis2D->Location);
				gp_XY xDir(1, 0);
				if (axis2D->RefDirection != nullptr)
					xDir = gp_XY(axis2D->RefDirection->DirectionRatios[0], axis2D->RefDirection->DirectionRatios[1]);
				return Ptr()->ToLocation(loc, xDir);
			}
			gp_Trsf GeometryProcedures::ToTransform(XbimMatrix3D m3D)
			{
				gp_Trsf trsf;
				trsf.SetValues(m3D.M11, m3D.M21, m3D.M31, m3D.OffsetX,
					m3D.M12, m3D.M22, m3D.M32, m3D.OffsetY,
					m3D.M13, m3D.M23, m3D.M33, m3D.OffsetZ);
				return trsf;
			}
			bool GeometryProcedures::IsFacingAwayFrom(IXFace^ face, IXDirection^ direction)
			{
				if (direction->IsNull) return false;
				gp_Vec toward(direction->X, direction->Y, direction->Z);
				const TopoDS_Face& topoFace = TopoDS::Face(((XFace^)face)->GetTopoShape());
				BRepGProp_Face prop(topoFace);
				gp_Pnt centre;
				gp_Vec faceNormal;
				double u1, u2, v1, v2;
				prop.Bounds(u1, u2, v1, v2);
				prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, faceNormal);

				double angle = faceNormal.AngleWithRef(toward, toward);
				return angle < M_PI_2 - 0.1;
			}

			IXPlane^ GeometryProcedures::BuildPlane(IIfcPlane^ plane)
			{
				gp_Ax2 axis = BuildAxis2Placement(plane->Position);
				gp_Ax3 ax3(axis);
				Handle(Geom_Plane) geomPlane = new Geom_Plane(ax3);
				return gcnew XPlane(geomPlane);
			}
			double GeometryProcedures::Distance(IXPoint^ a, IXPoint^ b)
			{
				gp_Pnt aPnt(a->X, a->Y, a->Z);
				gp_Pnt bPnt(b->X, b->Y, b->Z);
				return aPnt.Distance(bPnt);
			}
			double GeometryProcedures::IsEqual(IXPoint^ a, IXPoint^ b, double tolerance)
			{
				gp_Pnt aPnt(a->X, a->Y, a->Z);
				gp_Pnt bPnt(b->X, b->Y, b->Z);
				return aPnt.IsEqual(bPnt, tolerance);
			}
			IXDirection^ GeometryProcedures::NormalAt(IXFace^ face, IXPoint^ position, double tolerance)
			{
				gp_Pnt pt(position->X, position->Y, position->Z);
				const TopoDS_Face& topoFace = TOPO_FACE(face);
				Handle(Geom_Surface) surface = BRep_Tool::Surface(topoFace);
				ShapeAnalysis_Surface sas(surface);
				// get UV of point on surface
				gp_Pnt2d uv = sas.ValueOfUV(pt, ModelService->Precision);
				GeomLProp_SLProps props(surface, uv.X(), uv.Y(), 1, tolerance);
				gp_Dir normal = props.Normal();
				return gcnew Xbim::Geometry::BRep::XDirection(normal);
			}
			Xbim::Geometry::Abstractions::IXVisualMaterial^ GeometryProcedures::BuildVisualMaterial(System::String^ name, Xbim::Ifc4::Interfaces::IIfcSurfaceStyleElementSelect^ styling)
			{
				return gcnew VisualMaterial(name, styling);
			}
			Xbim::Geometry::Abstractions::IXVisualMaterial^ GeometryProcedures::BuildVisualMaterial(System::String^ name)
			{
				return gcnew VisualMaterial(name);
			}
			Xbim::Geometry::Abstractions::IXColourRGB^ GeometryProcedures::BuildColourRGB(double red, double green, double blue)
			{
				return gcnew ColourRGB(red, green, blue);
			}
			Xbim::Geometry::Abstractions::IXShapeColour^ GeometryProcedures::BuildShapeColour(System::String^ name, IIfcSurfaceStyleElementSelect^ surfaceStyle)
			{
				return gcnew XbimShapeColour(name,surfaceStyle);
			}
			Xbim::Geometry::Abstractions::IXLocation^ GeometryProcedures::BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz)
			{
				return gcnew Xbim::Geometry::BRep::XLocation(tx, ty, tz, sc, qw, qx, qy, qz);
			}
		}
	}
}