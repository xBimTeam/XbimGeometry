///TODO: Replace all exceptions with RaisGeoemtryFactoryException
//fix the raising of excpetions for vectors and directions 

#include "GeometryFactory.h"
#include "../Services//ModelService.h"
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
#include <gp_Dir2d.hxx>
#include <TColgp_Array1OfPnt2d.hxx>

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

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{

			IXDirection^ GeometryFactory::BuildDirection3d(double x, double y, double z)
			{
				return gcnew Xbim::Geometry::BRep::XDirection(x, y, z);
			};
			IXDirection^ GeometryFactory::BuildDirection2d(double x, double y)
			{
				return gcnew Xbim::Geometry::BRep::XDirection(x, y);
			};

			IXPoint^ GeometryFactory::BuildPoint3d(double x, double y, double z)
			{
				return gcnew XPoint(x, y, z);
			};
			IXPoint^ GeometryFactory::BuildPoint2d(double x, double y)
			{
				return gcnew XPoint(x, y);
			};

			

			gp_Pnt GeometryFactory::BuildPoint3d(IIfcCartesianPoint^ ifcPoint)
			{
				return gp_Pnt(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1], (int)ifcPoint->Dim == 3 ? (double)ifcPoint->Coordinates[2] : 0.0);
			}

			gp_Pnt GeometryFactory::BuildPoint3d(IXPoint^ xPoint)
			{
				return gp_Pnt(xPoint->X, xPoint->Y, (int)xPoint->Is3d ? xPoint->Z : 0.0);
			}

			bool GeometryFactory::BuildPoint2d(IIfcCartesianPoint^ ifcPoint, gp_Pnt2d& pnt2d)
			{
				if ((int)ifcPoint->Dim == 2)
				{
					pnt2d.SetXY(gp_XY(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1]));
					return true;
				}
				else
					return false;
			}

			gp_Pnt2d GeometryFactory::BuildPoint2d(IIfcCartesianPoint^ ifcPoint)
			{
				return gp_Pnt2d(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1]);
			}

			bool GeometryFactory::BuildDirection2d(IIfcDirection^ ifcDir, gp_Vec2d& dir2d)
			{
				if ((int)ifcDir->Dim != 2) return false;
				return EXEC_NATIVE->BuildDirection2d(ifcDir->DirectionRatios[0], ifcDir->DirectionRatios[1], dir2d);
			}


			bool GeometryFactory::BuildDirection3d(IIfcDirection^ ifcDir, gp_Vec& dir)
			{
				return EXEC_NATIVE->BuildDirection3d(ifcDir->DirectionRatios[0], ifcDir->DirectionRatios[1], ifcDir->DirectionRatios[2], dir);
			}

			bool GeometryFactory::BuildVector3d(IIfcVector^ ifcVec, gp_Vec& vec)
			{
				if ((double)ifcVec->Magnitude <= 0) return false;
				if (BuildDirection3d(ifcVec->Orientation, vec))
				{
					vec *= (double)ifcVec->Magnitude;
					return true;
				}
				else
					return false;
			}

			bool GeometryFactory::BuildVector2d(IIfcVector^ ifcVec, gp_Vec2d& vec)
			{
				if ((double)ifcVec->Magnitude <= 0) return false;
				if (BuildDirection2d(ifcVec->Orientation, vec))
				{
					vec *= (double)ifcVec->Magnitude;
					return true;
				}
				else
					return false;
			}

			bool GeometryFactory::BuildAxis2Placement3d(IIfcAxis2Placement3D^ axis2, gp_Ax2& ax2)
			{
				if (axis2->Axis == nullptr || axis2->RefDirection == nullptr) //both have to be given if one is null use the defaults
				{
					if (EXEC_NATIVE->BuildAxis2Placement3d(BuildPoint3d(axis2->Location), gp::DZ(), gp::DX(), ax2))
						return true;
				}
				else
				{
					gp_Vec axis;
					gp_Vec refDir;
					if (!BuildDirection3d(axis2->Axis, axis)) return false;
					if (!BuildDirection3d(axis2->RefDirection, refDir)) return false;
					if (EXEC_NATIVE->BuildAxis2Placement3d(BuildPoint3d(axis2->Location), axis, refDir, ax2))
						return true;
				}
				return false;
			}

			bool GeometryFactory::BuildAxis2Placement2d(IIfcAxis2Placement2D^ axis2d, gp_Ax22d& ax22)
			{
				gp_Pnt2d loc;
				if (!BuildPoint2d(axis2d->Location, loc))
					return false;
				if (axis2d->RefDirection == nullptr) //both have to be given if one is null use the defaults
				{
					return EXEC_NATIVE->BuildAxis2Placement2d(loc, gp::DX2d(), ax22);
				}
				else
				{
					gp_Vec2d refDir;
					if (!BuildDirection2d(axis2d->RefDirection, refDir)) return false;
					return EXEC_NATIVE->BuildAxis2Placement2d(loc, refDir, ax22);
				}
			}

			IXAxis2Placement2d^ GeometryFactory::GetAxis2Placement2d(IXPoint^ location, IXVector^ XaxisDirection)
			{
				gp_Ax2d axis = gp_Ax2d(
					gp_Pnt2d(location->X, location->Y),
					gp_Dir2d(XaxisDirection->X, XaxisDirection->Y)
				);
				Handle(Geom2d_AxisPlacement) hPlacement = new Geom2d_AxisPlacement(axis);
				return gcnew XAxisPlacement2d(hPlacement);
			}

			void GeometryFactory::GetPolylinePoints3d(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt& points)
			{
				
				int i = 1;
				for each (IIfcCartesianPoint ^ ifcPoint in ifcPolyline->Points)
				{
					gp_Pnt pnt = BuildPoint3d(ifcPoint);
					points.SetValue(i, pnt);
					i++;
				}
				
			}

			void GeometryFactory::GetPolylinePoints2d(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt2d& points)
			{
				
				int i = 1;
				for each (IIfcCartesianPoint ^ ifcPoint in ifcPolyline->Points)
				{
					gp_Pnt2d pnt2d;
					if (!BuildPoint2d(ifcPoint, pnt2d))
						throw RaiseGeometryFactoryException("Polyline points must all be 2d", ifcPolyline);
					points.SetValue(i, pnt2d);
					i++;
				}
				
			}

			bool GeometryFactory::ToLocation(IIfcAxis2Placement2D^ axis2D, TopLoc_Location& location)
			{
				gp_Pnt2d pnt2d;
				if (!BuildPoint2d(axis2D->Location, pnt2d))
					throw RaiseGeometryFactoryException("IIfcAxis2Placement2D Location must be 2D", axis2D);
				gp_XY xDir(1, 0);
				if (axis2D->RefDirection != nullptr)
					xDir = gp_XY(axis2D->RefDirection->DirectionRatios[0], axis2D->RefDirection->DirectionRatios[1]);
				return EXEC_NATIVE->ToLocation(pnt2d, xDir, location);
			}

			gp_Trsf GeometryFactory::ToTransform(XbimMatrix3D m3D)
			{
				gp_Trsf trsf;
				trsf.SetValues(m3D.M11, m3D.M21, m3D.M31, m3D.OffsetX,
					m3D.M12, m3D.M22, m3D.M32, m3D.OffsetY,
					m3D.M13, m3D.M23, m3D.M33, m3D.OffsetZ);
				return trsf;
			}
			bool GeometryFactory::IsFacingAwayFrom(IXFace^ face, IXDirection^ direction)
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

			IXPlane^ GeometryFactory::BuildPlane(IIfcPlane^ plane)
			{
				gp_Ax2 axis;
				if(!BuildAxis2Placement3d(plane->Position,axis))
					throw RaiseGeometryFactoryException("IIfcAxis2Placement2D Location must be 2D", plane->Position);
				gp_Ax3 ax3(axis);
				Handle(Geom_Plane) geomPlane = new Geom_Plane(ax3);
				return gcnew XPlane(geomPlane);
			}
			double GeometryFactory::Distance(IXPoint^ a, IXPoint^ b)
			{
				gp_Pnt aPnt(a->X, a->Y, a->Z);
				gp_Pnt bPnt(b->X, b->Y, b->Z);
				return aPnt.Distance(bPnt);
			}
			double GeometryFactory::IsEqual(IXPoint^ a, IXPoint^ b, double tolerance)
			{
				gp_Pnt aPnt(a->X, a->Y, a->Z);
				gp_Pnt bPnt(b->X, b->Y, b->Z);
				return aPnt.IsEqual(bPnt, tolerance);
			}
			IXDirection^ GeometryFactory::NormalAt(IXFace^ face, IXPoint^ position, double tolerance)
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

			Xbim::Geometry::Abstractions::IXLocation^ GeometryFactory::BuildLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz)
			{
				return gcnew Xbim::Geometry::BRep::XLocation(tx, ty, tz, sc, qw, qx, qy, qz);
			}

			double GeometryFactory::GetDeterminant(double x1, double y1, double x2, double y2)
			{
				return x1 * y2 - x2 * y1;
			}

			double GeometryFactory::Area(const TColgp_SequenceOfPnt2d& points2d)
			{
				if (points2d.Length() < 3)
				{
					return 0;
				}
				int nbPoints = (int)points2d.Length();
				double area = GetDeterminant(points2d.Value(nbPoints).X(), points2d.Value(nbPoints).Y(), points2d.Value(1).X(), points2d.Value(1).Y());

				for (int i = 2; i <= nbPoints; i++)
				{
					area += GetDeterminant(points2d.Value(i - 1).X(), points2d.Value(i - 1).Y(), points2d.Value(i).X(), points2d.Value(i).Y());
				}
				return (area / 2);
			}

		}
	}
}