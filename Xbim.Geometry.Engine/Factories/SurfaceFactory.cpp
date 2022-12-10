#include "SurfaceFactory.h"
#include "GeometryFactory.h"
#include "CurveFactory.h"
#include "ProfileFactory.h"
#include "BIMAuthoringToolWorkArounds.h"
#include "EdgeFactory.h"
#include <Geom_Plane.hxx>
#include "../BRep//XPlane.h"
#include <TopoDS_Edge.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>

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
				throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
				return Handle(Geom_Surface)();
			}


			Handle(Geom_Plane) SurfaceFactory::BuildPlane(IIfcPlane^ ifcPlane)
			{
				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint3d(ifcPlane->Position->Location);
				gp_Vec normal;
				if (!GEOMETRY_FACTORY->BuildDirection3d(ifcPlane->Position->Axis, normal))
					throw RaiseGeometryFactoryException("Plane axis is incorrectly defined", ifcPlane->Position->Axis);
				Handle(Geom_Plane) plane = OccHandle().BuildPlane(origin, normal);
				if (plane.IsNull())
					throw RaiseGeometryFactoryException("Plane is badly defined. See logs", ifcPlane);
				return plane;
			}

			Handle(Geom_Surface) SurfaceFactory::BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcSurfaceOfRevolution)
			{
				throw RaiseGeometryFactoryException("BuildSurfaceOfRevolution is not implemented");
				//if (ifcSurfaceOfRevolution->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
				//	throw RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of revolution.", ifcSurfaceOfRevolution->SweptCurve);

				//
				//TopoDS_Edge sweptEdge = EDGE_FACTORY->BuildEdge(ifcSurfaceOfRevolution->SweptCurve); //throws exception


				//TopoDS_Edge startEdge = sweptEdge;

				//gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint3d(ifcSurfaceOfRevolution->AxisPosition->Location);
				//gp_Vec axisDir(0, 0, 1);
				//if (ifcSurfaceOfRevolution->AxisPosition->Axis != nullptr)
				//{
				//	if (!GEOMETRY_FACTORY->BuildDirection3d(ifcSurfaceOfRevolution->Position->Axis, axisDir))
				//		throw RaiseGeometryFactoryException("IIfcSurfaceOfRevolution axis is incorrectly defined", ifcSurfaceOfRevolution->Position->Axis);
				//}
				//gp_Ax1 axis(origin, axisDir);

				//BRepPrimAPI_MakeRevol revolutor(startEdge, axis, M_PI * 2);
				//if (!revolutor.IsDone() && revolutor.Shape().ShapeType() == TopAbs_FACE)
				//{
				//	TopoDS_Face face = TopoDS::Face(revolutor.Shape());
				//	return BRep_Tool::Surface(face);
				//}
				//else
				//{
				//	throw RaiseGeometryFactoryException("Invalid IfcSurfaceOfRevolution", ifcSurfaceOfRevolution);
				//}
				return Handle(Geom_Surface)();
			}

			Handle(Geom_Surface) SurfaceFactory::BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion)
			{
				if (ifcSurfaceOfLinearExtrusion->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
					throw RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of linearExtrusion", ifcSurfaceOfLinearExtrusion);
				TopoDS_Edge sweptEdge;
				if (!BIM_WORKAROUNDS->FixRevitIncorrectArcCentreSweptCurve(ifcSurfaceOfLinearExtrusion, sweptEdge))
				{
					//didn't need a fix so just create it
					sweptEdge = PROFILE_FACTORY->BuildProfileEdge(ifcSurfaceOfLinearExtrusion->SweptCurve);
				}
				gp_Vec extrude;
				if (!GEOMETRY_FACTORY->BuildDirection3d(ifcSurfaceOfLinearExtrusion->ExtrudedDirection, extrude))
					throw RaiseGeometryFactoryException("Direction of IfcSurfaceOfLinearExtrusion is invalid", ifcSurfaceOfLinearExtrusion->ExtrudedDirection);
				extrude *= ifcSurfaceOfLinearExtrusion->Depth;
				BIM_WORKAROUNDS->FixRevitSweptSurfaceExtrusionInFeet(extrude);
				BIM_WORKAROUNDS->FixRevitIncorrectBsplineSweptCurve(ifcSurfaceOfLinearExtrusion, sweptEdge);

				Handle(Geom_SurfaceOfLinearExtrusion) surface = OccHandle().BuildSurfaceOfLinearExtrusion(sweptEdge, extrude);
				if(surface.IsNull())
					throw RaiseGeometryFactoryException("Surface of IfcSurfaceOfLinearExtrusion is invalid", ifcSurfaceOfLinearExtrusion);			
				return surface;
			}

			Handle(Geom_Plane) SurfaceFactory::BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane)
			{
				throw gcnew NotImplementedException();
			}
			Handle(Geom_Surface) SurfaceFactory::BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface)
			{
				throw gcnew NotImplementedException();
			}
			Handle(Geom_Surface) SurfaceFactory::BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface)
			{
				throw gcnew NotImplementedException();
			}
			Handle(Geom_Surface) SurfaceFactory::BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots)
			{
				throw gcnew NotImplementedException();
			}
			Handle(Geom_Surface) SurfaceFactory::BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots)
			{
				throw gcnew NotImplementedException();
			}
			Handle(Geom_Surface) SurfaceFactory::BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface)
			{
				throw gcnew NotImplementedException();
			}

#pragma endregion
		}
	}
}
