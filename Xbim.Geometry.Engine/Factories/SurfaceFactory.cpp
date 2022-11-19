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
			Handle(Geom_Surface) SurfaceFactory::BuildOccSurface(IIfcSurface^ ifcSurface)
			{
				if (dynamic_cast<IIfcPlane^>(ifcSurface))
					return BuildOccPlane((IIfcPlane^)ifcSurface);
				else if (dynamic_cast<IIfcSurfaceOfRevolution^>(ifcSurface))
					return BuildOccSurfaceOfRevolution((IIfcSurfaceOfRevolution^)ifcSurface);
				else if (dynamic_cast<IIfcSurfaceOfLinearExtrusion^>(ifcSurface))
					return BuildOccSurfaceOfLinearExtrusion((IIfcSurfaceOfLinearExtrusion^)ifcSurface);
				else if (dynamic_cast<IIfcCurveBoundedPlane^>(ifcSurface))
					return BuildOccCurveBoundedPlane((IIfcCurveBoundedPlane^)ifcSurface);
				else if (dynamic_cast<IIfcRectangularTrimmedSurface^>(ifcSurface))
					return BuildOccRectangularTrimmedSurface((IIfcRectangularTrimmedSurface^)ifcSurface);
				else if (dynamic_cast<IIfcBSplineSurface^>(ifcSurface))
					return BuildOccBSplineSurface((IIfcBSplineSurface^)ifcSurface);
				else if (dynamic_cast<IIfcCylindricalSurface^>(ifcSurface))
					return BuildOccCylindricalSurface((IIfcCylindricalSurface^)ifcSurface);
				else
				{
					System::Type^ type = ifcSurface->GetType();
					throw(gcnew System::NotImplementedException(System::String::Format("XbimFace. BuildSurfaceof type {0} is not implemented", type->Name)));
				}
			}


			Handle(Geom_Surface) SurfaceFactory::BuildOccPlane(IIfcPlane^ ifcPlane)
			{
				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint(ifcPlane->Position->Location);
				gp_Dir normal = GEOMETRY_FACTORY->BuildDirection(ifcPlane->Position->Axis);
				return EXEC_NATIVE->BuildPlane(origin, normal);
			}

			Handle(Geom_Surface) SurfaceFactory::BuildOccSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcSurfaceOfRevolution)
			{
				if (ifcSurfaceOfRevolution->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
					RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of revolution.", ifcSurfaceOfRevolution->SweptCurve);
				

				TopoDS_Edge sweptEdge = CURVE_FACTORY->BuildProfileDef(ifcSurfaceOfRevolution->SweptCurve);
				
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

			Handle(Geom_Surface) SurfaceFactory::BuildOccSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion)
			{

			}

			Handle(Geom_Surface) SurfaceFactory::BuildOccCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane)
			{

			}

			Handle(Geom_Surface) SurfaceFactory::BuildOccRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface)
			{

			}
			Handle(Geom_Surface) SurfaceFactory::BuildOccBSplineSurface(IIfcBSplineSurface^ ifcBSplineSurface)
			{

			}
			Handle(Geom_Surface) SurfaceFactory::BuildOccCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface)
			{

			}

#pragma endregion
		}
	}
}
