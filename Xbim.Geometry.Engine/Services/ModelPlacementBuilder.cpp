
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

				IIfcLocalPlacement^ localPlacement = dynamic_cast<IIfcLocalPlacement^>(placement);
				if (localPlacement == nullptr) 
					return gcnew XLocation();

				return gcnew XLocation(TopLoc_Location(ToTransform(localPlacement, adjustWcs)));
			}

			gp_Trsf ModelPlacementBuilder::ToTransform(IIfcLocalPlacement^ localPlacement, bool adjustWcs)
			{
				 
				gp_Trsf trsf;
				// multiply the transformations up to the root
				int rootId = adjustWcs? _rootId : -1;
				while (localPlacement != nullptr && localPlacement->EntityLabel != rootId) 
				{
					 
					IIfcAxis2Placement3D^ axisPlacement3D = dynamic_cast<IIfcAxis2Placement3D^>(localPlacement->RelativePlacement);

					if (axisPlacement3D != nullptr)
					{
						gp_Trsf relTrsf;
						gp_Pnt p = BuildPoint3d(axisPlacement3D->Location);
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
					else
					{
						throw RaiseGeometryFactoryException("Support for Object Placements other than 3D not implemented");
					}
					localPlacement = dynamic_cast<IIfcLocalPlacement^>(localPlacement->PlacementRelTo);
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
			 
		}
	}
}