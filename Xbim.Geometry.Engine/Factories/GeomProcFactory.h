//Geometric Processor Factory, builds  entities used for algebraic calculation etc from their Ifc counterparts
#pragma once
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>

#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class GeomProcFactory
			{
			public:
				GeomProcFactory()
				{

				}
				//builds a 3d point, if the Ifc point is 2d the Z coordinate is 0
				gp_Pnt BuildPoint(IIfcCartesianPoint^ ifcPoint)
				{
					return gp_Pnt(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1], (int)ifcPoint->Dim == 3 ? (double)ifcPoint->Coordinates[2] : 0.0);
				}
				//builds a 2d point, if the Ifc point is 3d an exception is thrown
				gp_Pnt2d BuildPoint2d(IIfcCartesianPoint^ ifcPoint)
				{
					if ((int)ifcPoint->Dim == 3) throw gcnew XbimGeometryFactoryException("Illegal attempt to build a 2d point from a 3d point. ");
					return gp_Pnt2d(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1]);
				}
				//builds a 3d direction, if the Ifc Direction is 2d the Z component is 0
				gp_Dir BuildDirection(IIfcDirection^ ifcDir)
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
				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				gp_Dir2d BuildDirection2d(IIfcDirection^ ifcDir)
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
				//builds a 3d vector, if the Ifc Vector is 2d the Z component is 0
				gp_Vec BuildVector(IIfcVector^ ifcVec)
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
				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				gp_Vec2d BuildVector2d(IIfcVector^ ifcVec)
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
			};
		}
	}
}
