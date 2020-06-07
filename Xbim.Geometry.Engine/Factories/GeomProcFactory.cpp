#include "GeomProcFactory.h"
#include <gp_Ax2.hxx>
#include <TColgp_SequenceOfPnt2d.hxx>
#include <TColgp_Array1OfPnt.hxx>

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			gp_Pnt GeomProcFactory::BuildPoint(IIfcCartesianPoint^ ifcPoint)
			{
				return gp_Pnt(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1], (int)ifcPoint->Dim == 3 ? (double)ifcPoint->Coordinates[2] : 0.0);
			}

			gp_Pnt2d GeomProcFactory::BuildPoint2d(IIfcCartesianPoint^ ifcPoint)
			{
				if ((int)ifcPoint->Dim == 3) throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d point from a 3d point. ");
				return gp_Pnt2d(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1]);
			}

			gp_Dir GeomProcFactory::BuildDirection(IIfcDirection^ ifcDir)
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

			gp_Dir2d GeomProcFactory::BuildDirection2d(IIfcDirection^ ifcDir)
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
			gp_Vec GeomProcFactory::BuildVector(IIfcVector^ ifcVec)
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

			gp_Vec2d GeomProcFactory::BuildVector2d(IIfcVector^ ifcVec)
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

			gp_Ax2 GeomProcFactory::BuildAxis2Placement(IIfcAxis2Placement3D^ axis2)
			{
				if (axis2->Axis == nullptr || axis2->RefDirection == nullptr) //both have to be given if one is null use the defaults
					return gp_Ax2(BuildPoint(axis2->Location), gp::DZ(), gp::DX());
				else return gp_Ax2(
					BuildPoint(axis2->Location),
					BuildDirection(axis2->Axis),
					BuildDirection(axis2->RefDirection)
				);
			}

			gp_Ax2d GeomProcFactory::BuildAxis2Placement2d(IIfcAxis2Placement2D^ axis2d)
			{
				if (axis2d->RefDirection == nullptr)  
					return gp_Ax2d( BuildPoint2d(axis2d->Location), gp::DX2d());
				else
					return gp_Ax2d(
					BuildPoint2d(axis2d->Location),
					BuildDirection2d(axis2d->RefDirection)
				);
			}
			
			void GeomProcFactory::GetPolylinePoints(IIfcPolyline^ ifcPolyline, TColgp_Array1OfPnt& points)
			{
				int i = 1;
				for each (IIfcCartesianPoint ^ ifcPoint in ifcPolyline->Points)
				{
					gp_Pnt pnt = BuildPoint(ifcPoint);
					points.SetValue(i, pnt);
					i++;
				}
			}
		}
	}
}