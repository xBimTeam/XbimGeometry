#include "SurfaceFactory.h"
#include "GeometryFactory.h"
#include "CurveFactory.h"
#include <Geom_Plane.hxx>
#include "../BRep//XPlane.h"
#include <TopoDS_Edge.hxx>

using namespace Xbim::Geometry::BRep;

using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
#pragma region Interfaces 


			IXPlane^ SurfaceFactory::BuildPlane(IXPoint^ origin, IXDirection^ normal)
			{
				Handle(Geom_Plane) occPlane = Ptr()->BuildPlane(origin->X, origin->Y, origin->Z, normal->X, normal->Y, normal->Z);
				if (occPlane.IsNull())
					throw gcnew Exception("Invalid arguments for plane");
				else
					return gcnew XPlane(occPlane);
			}

#pragma endregion

#pragma region OCC
			Handle(Geom_Surface) SurfaceFactory::BuildSurface(IIfcSurface^ ifcSurface)
			{
				if (dynamic_cast<IIfcPlane^>(ifcSurface))
					return BuildPlane((IIfcPlane^)ifcSurface);
				else if (dynamic_cast<IIfcSurfaceOfRevolution^>(ifcSurface))
					return BuildSurfaceOfRevolution((IIfcSurfaceOfRevolution^)ifcSurface);
				else if (dynamic_cast<IIfcSurfaceOfLinearExtrusion^>(ifcSurface))
					return BuildSurfaceOfLinearExtrusion((IIfcSurfaceOfLinearExtrusion^)ifcSurface);
				else if (dynamic_cast<IIfcCurveBoundedPlane^>(ifcSurface))
					return BuildCurveBoundedPlane((IIfcCurveBoundedPlane^)ifcSurface);
				else if (dynamic_cast<IIfcCurveBoundedSurface^>(ifcSurface))
					return BuildCurveBoundedSurface((IIfcCurveBoundedSurface^)ifcSurface);
				else if (dynamic_cast<IIfcRectangularTrimmedSurface^>(ifcSurface))
					return BuildRectangularTrimmedSurface((IIfcRectangularTrimmedSurface^)ifcSurface);
				else if (dynamic_cast<IIfcBSplineSurfaceWithKnots^>(ifcSurface))
					return BuildBSplineSurfaceWithKnots((IIfcBSplineSurfaceWithKnots^)ifcSurface);
				else if (dynamic_cast<IIfcRationalBSplineSurfaceWithKnots^>(ifcSurface))
					return BuildRationalBSplineSurfaceWithKnots((IIfcRationalBSplineSurfaceWithKnots^)ifcSurface);
				else if (dynamic_cast<IIfcCylindricalSurface^>(ifcSurface))
					return BuildCylindricalSurface((IIfcCylindricalSurface^)ifcSurface);
				else			
					RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
			}


			Handle(Geom_Plane) SurfaceFactory::BuildPlane(IIfcPlane^ ifcPlane)
			{
				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint(ifcPlane->Position->Location);
				gp_Dir normal = GEOMETRY_FACTORY->BuildDirection(ifcPlane->Position->Axis);
				Handle(Geom_Plane) plane =  EXEC_NATIVE->BuildPlane(origin, normal);
				if (plane.IsNull())
					RaiseGeometryFactoryException("Plane is badly defined. See logs", ifcPlane);
			}

			Handle(Geom_Surface) SurfaceFactory::BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcSurfaceOfRevolution)
			{
				if (ifcSurfaceOfRevolution->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
					RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of revolution.", ifcSurfaceOfRevolution->SweptCurve);
				

				TopoDS_Edge sweptEdge = PROFILE_FACTORY->BuildProfileDef(ifcSurfaceOfRevolution->SweptCurve);
				
				if (!edge->IsValid)
				{
					XbimGeometryCreator::LogWarning(logger, sRev, "Invalid Swept Curve for IfcSurfaceOfRevolution, face discarded");
					return;
				}
				TopoDS_Edge startEdge = edge;

				gp_Pnt origin(sRev->AxisPosition->Location->X, sRev->AxisPosition->Location->Y, sRev->AxisPosition->Location->Z);
				gp_Dir axisDir(0, 0, 1);
				if (sRev->AxisPosition->Axis != nullptr)
					axisDir = gp_Dir(sRev->AxisPosition->Axis->X, sRev->AxisPosition->Axis->Y, sRev->AxisPosition->Axis->Z);
				gp_Ax1 axis(origin, axisDir);

				BRepPrimAPI_MakeRevol revolutor(startEdge, axis, M_PI * 2);
				if (revolutor.IsDone())
				{
					pFace = new TopoDS_Face();
					*pFace = TopoDS::Face(revolutor.Shape());
					pFace->EmptyCopy();
				}
				else
				{
					XbimGeometryCreator::LogWarning(logger, sRev, "Invalid IfcSurfaceOfRevolution, face discarded");
				}

			}

			Handle(Geom_Surface) SurfaceFactory::BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion)
			{

			}

			Handle(Geom_Plane) SurfaceFactory::BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane)
			{

			}
Handle(Geom_Surface) SurfaceFactory::BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface)
			{

			}
			Handle(Geom_Surface) SurfaceFactory::BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface)
			{

			}
			Handle(Geom_Surface) SurfaceFactory::BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots)
			{

			}
			Handle(Geom_Surface) SurfaceFactory::BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots)
			{

			}
			Handle(Geom_Surface) SurfaceFactory::BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface)
			{

			}

#pragma endregion
		}
	}
}
