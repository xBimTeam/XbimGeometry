
#include "ModelPlacementBuilder.h"
#include "../Factories/GeometryFactory.h"
#include "../BRep/XLocation.h"
#include <gp_Ax3.hxx>

using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::BRep;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			 
			IXLocation^ ModelPlacementBuilder::BuildLocation(IIfcObjectPlacement^ placement, bool adjustWcs) {

				IIfcGridPlacement^ gridPlacement = dynamic_cast<IIfcGridPlacement^>(placement);
				if (gridPlacement != nullptr) {

					return this->_modelGeometryServices->GeometryFactory->BuildLocation(gridPlacement);
				}

				return gcnew XLocation(TopLoc_Location(ToTransform(placement, adjustWcs)));
			}

			gp_Trsf ModelPlacementBuilder::ToTransform(IIfcObjectPlacement^ placement, bool adjustWcs)
			{

				IIfcLocalPlacement^ localPlacement = dynamic_cast<IIfcLocalPlacement^>(placement);
				Xbim::Ifc4x3::GeometricConstraintResource::IfcLinearPlacement^ linearPlacement =
					dynamic_cast<Xbim::Ifc4x3::GeometricConstraintResource::IfcLinearPlacement^>(placement);
				gp_Trsf trsf;

				// multiply the transformations up to the root
				int rootId = adjustWcs? _rootId : -1;
				while (localPlacement != nullptr || linearPlacement != nullptr)
				{
					if (localPlacement != nullptr)
					{
						IIfcAxis2Placement3D^ axisPlacement3D = dynamic_cast<IIfcAxis2Placement3D^>(localPlacement->RelativePlacement);
						if (axisPlacement3D) {
							gp_Trsf relTrsf;
							gp_Pnt p = localPlacement->EntityLabel == rootId ?
								gp_Pnt(0, 0, 0) :
								BuildPoint3d(axisPlacement3D->Location);
							if (axisPlacement3D->RefDirection != nullptr && axisPlacement3D->Axis != nullptr)
							{
								gp_Vec axis, refDir;
								if (!BuildDirection3d(axisPlacement3D->Axis, axis))
									throw RaiseGeometryFactoryException("IfcObjectPlacement axis direction is illegal", axisPlacement3D->Axis);
								if (!BuildDirection3d(axisPlacement3D->RefDirection, refDir))
									throw RaiseGeometryFactoryException("IfcObjectPlacement reference direction is illegal", axisPlacement3D->Axis);
								gp_Ax3 ax3(p, axis, refDir);
								relTrsf.SetTransformation(ax3, gp_Ax3(gp_Pnt(), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)));
							}
							else
							{
								gp_Ax3 ax3(p, gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
								relTrsf.SetTransformation(ax3, gp_Ax3(gp_Pnt(), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)));
							}

							trsf.PreMultiply(relTrsf);
						}
						else {
							throw RaiseGeometryFactoryException("Support for Object Placements other than 3D not implemented");
						}

						EvaluateNextPlacement(localPlacement, linearPlacement);
					}
					else if (linearPlacement != nullptr)
					{
						Xbim::Ifc4x3::GeometryResource::IfcAxis2PlacementLinear^ axislinearPlacement
							= dynamic_cast<Xbim::Ifc4x3::GeometryResource::IfcAxis2PlacementLinear^>(localPlacement->RelativePlacement);
						
						gp_Trsf relTrsf;

						IfcPointByDistanceExpression^ point = dynamic_cast<IfcPointByDistanceExpression^>(axislinearPlacement->Location);
						
						if (point == nullptr)
							throw RaiseGeometryFactoryException("IfcAxis2PlacementLinear should have a Location property of type IfcPointByDistanceExpression", axislinearPlacement);

						gp_Pnt loc;
						gp_Vec tangent;
						gp_Vec axis;
						Standard_Real distancesAlong;

						if (GEOMETRY_FACTORY->BuildPoint3d(point, loc, tangent, axis, distancesAlong))
						{
							if (linearPlacement->EntityLabel == rootId) 
							{
								loc = gp_Pnt(0, 0, 0);
							}

							if (axislinearPlacement->Axis != nullptr && axislinearPlacement->RefDirection != nullptr)
							{
								gp_Vec zDir;
								if (!BuildDirection3d(axislinearPlacement->Axis, zDir))
									throw RaiseGeometryFactoryException("IfcAxis2PlacementLinear Axis Direction is invalid ", axislinearPlacement->Axis);
								zDir.Normalize();
								gp_Vec xDir;
								if (!BuildDirection3d(axislinearPlacement->RefDirection, xDir))
									throw RaiseGeometryFactoryException("IfcAxis2PlacementLinear Reference Direction is invalid ", axislinearPlacement->RefDirection);
								xDir.Normalize();
								relTrsf.SetTransformation(gp_Ax3(loc, zDir, xDir), gp_Ax3(gp_Pnt(), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)));
							}
							else
							{
								gp_Dir up(0, 0, 1);
								gp_Dir axis = tangent.Crossed(up.Crossed(tangent));
								relTrsf.SetTransformation(gp_Ax3(loc, axis, tangent), gp_Ax3(gp_Pnt(), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)));
							}

							trsf.PreMultiply(relTrsf);
						}
						else {
							throw RaiseGeometryFactoryException("Couldn't build IfcAxis2PlacementLinear", axislinearPlacement);
						}

						EvaluateNextPlacement(localPlacement, linearPlacement);
					}
					else
					{
						throw RaiseGeometryFactoryException("Support for Object Placements other than 3D not implemented");
					}
				}

				return trsf;
			}

			gp_Pnt ModelPlacementBuilder::BuildPoint3d(IIfcCartesianPoint^ ifcPoint)
			{
				return gp_Pnt(ifcPoint->Coordinates[0], ifcPoint->Coordinates[1], (int)ifcPoint->Dim == 3 ? (double)ifcPoint->Coordinates[2] : 0.0);
			}

			bool ModelPlacementBuilder::BuildDirection3d(IIfcDirection^ ifcDir, gp_Vec& dir)
			{

				return EXEC_NATIVE->BuildDirection3d(ifcDir->DirectionRatios[0], ifcDir->DirectionRatios[1], ifcDir->DirectionRatios[2], dir);
			}

			void ModelPlacementBuilder::EvaluateNextPlacement(IIfcLocalPlacement^% localPlacement, Xbim::Ifc4x3::GeometricConstraintResource::IfcLinearPlacement^% linearPlacement)
			{
				if (localPlacement != nullptr && localPlacement->PlacementRelTo != nullptr)
				{
					auto relTo = localPlacement->PlacementRelTo;

					IIfcLocalPlacement^ nextLocalPlacement = dynamic_cast<IIfcLocalPlacement^>(relTo);

					if (nextLocalPlacement != nullptr)
					{
						localPlacement = nextLocalPlacement;
						linearPlacement = nullptr;
					}
					else
					{
						linearPlacement = dynamic_cast<Xbim::Ifc4x3::GeometricConstraintResource::IfcLinearPlacement^>(relTo);

						if (linearPlacement != nullptr)
						{
							localPlacement = nullptr;
						}
						else
						{
							localPlacement = nullptr;
							linearPlacement = nullptr;
						}
					}
				}
				else
				{
					localPlacement = nullptr;
					linearPlacement = nullptr;
				}

			}
			 
		}
	}
}