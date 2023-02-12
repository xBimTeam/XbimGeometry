#include "SurfaceFactory.h"
#include "GeometryFactory.h"
#include "CurveFactory.h"
#include "WireFactory.h"
#include "ProfileFactory.h"
#include "BIMAuthoringToolWorkArounds.h"
#include "EdgeFactory.h"
#include "../BRep/XFace.h"
#include "../BRep/XPlane.h"
#include "../BRep/XCylindricalSurface.h"
#include "../BRep/XConicalSurface.h"
#include "../BRep/XSurfaceOfRevolution.h"
#include "../BRep/XSurfaceOfLinearExtrusion.h"

#include <TopoDS_Edge.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeFix_Edge.hxx>
#include <TopExp_Explorer.hxx>

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
					throw RaiseGeometryFactoryException("Error building plane");
				else
					return gcnew XPlane(occPlane);
			}

			IXSurface^ SurfaceFactory::Build(IIfcSurface^ ifcSurface)
			{
				XSurfaceType surfaceType;
				Handle(Geom_Surface) surface;
				auto curveBoundedPlane = dynamic_cast<IIfcCurveBoundedPlane^>(ifcSurface);
				auto curveBoundedSurface = dynamic_cast<IIfcCurveBoundedSurface^>(ifcSurface);
				if (curveBoundedPlane != nullptr)
				{
					TopoDS_Face face = BuildCurveBoundedPlane(curveBoundedPlane);
					if (!face.IsNull()) return gcnew XFace(face); else throw RaiseGeometryFactoryException("Failed to build surface", ifcSurface);
				}
				else if (curveBoundedSurface != nullptr)
				{
					TopoDS_Face face = BuildCurveBoundedSurface(curveBoundedSurface);
					if (!face.IsNull()) return gcnew XFace(face); else throw RaiseGeometryFactoryException("Failed to build surface", ifcSurface);
				}
				else
				{
					Handle(Geom_Surface) surface = BuildSurface(ifcSurface, surfaceType);
					if (surface.IsNull())
						throw RaiseGeometryFactoryException("Error building surface", ifcSurface);
					return XSurface::GeomToXSurface(surface);
				}
			}

#pragma endregion

#pragma region OCC
			Handle(Geom_Surface) SurfaceFactory::BuildSurface(IIfcSurface^ ifcSurface, XSurfaceType% surfaceType)
			{
				if (!Enum::TryParse<XSurfaceType>(ifcSurface->ExpressType->ExpressName, surfaceType))
					throw RaiseGeometryFactoryException("Unsupported surface type", ifcSurface);

				switch (surfaceType)
				{
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcBSplineSurfaceWithKnots:
					return BuildBSplineSurfaceWithKnots((IIfcBSplineSurfaceWithKnots^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcRationalBSplineSurfaceWithKnots:
					return BuildRationalBSplineSurfaceWithKnots((IIfcRationalBSplineSurfaceWithKnots^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcCurveBoundedPlane:
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcCurveBoundedSurface:
					throw RaiseGeometryFactoryException("Curve Bounded Surfaces must be built as faces", ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcRectangularTrimmedSurface:
					return BuildRectangularTrimmedSurface((IIfcRectangularTrimmedSurface^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcSurfaceOfLinearExtrusion:
					return BuildSurfaceOfLinearExtrusion((IIfcSurfaceOfLinearExtrusion^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcSurfaceOfRevolution:
					return BuildSurfaceOfRevolution((IIfcSurfaceOfRevolution^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcCylindricalSurface:
					return BuildCylindricalSurface((IIfcCylindricalSurface^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcPlane:
					return BuildPlane((IIfcPlane^)ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcSphericalSurface:
					throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
				case Xbim::Geometry::Abstractions::XSurfaceType::IfcToroidalSurface:
					throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
				default:
					throw RaiseGeometryFactoryException("Surface of type is not implemented", ifcSurface);
				}

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

			Handle(Geom_SurfaceOfRevolution) SurfaceFactory::BuildSurfaceOfRevolution(IIfcSurfaceOfRevolution^ ifcSurfaceOfRevolution)
			{
				if (ifcSurfaceOfRevolution->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
					throw RaiseGeometryFactoryException("Only profiles of type curve are valid in a surface of revolution.", ifcSurfaceOfRevolution->SweptCurve);
				XProfileDefType profileDefType;
				Handle(Geom_Curve) sweptEdge = PROFILE_FACTORY->BuildCurve(ifcSurfaceOfRevolution->SweptCurve, profileDefType); //throws exception
				if (sweptEdge.IsNull())
					throw RaiseGeometryFactoryException("IfcSurfaceOfRevolution swept edge is incorrectly defined", ifcSurfaceOfRevolution->SweptCurve);

				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint3d(ifcSurfaceOfRevolution->AxisPosition->Location);
				gp_Vec axisDir(0, 0, 1);
				if (ifcSurfaceOfRevolution->AxisPosition->Axis != nullptr)
				{
					if (!GEOMETRY_FACTORY->BuildDirection3d(ifcSurfaceOfRevolution->AxisPosition->Axis, axisDir))
						throw RaiseGeometryFactoryException("IfcSurfaceOfRevolution axis is incorrectly defined", ifcSurfaceOfRevolution->AxisPosition->Axis);
				}
				gp_Ax1 axis(origin, axisDir);
				Handle(Geom_SurfaceOfRevolution) revolutedSurface = new Geom_SurfaceOfRevolution(sweptEdge, axis);

				if (revolutedSurface.IsNull())
					throw RaiseGeometryFactoryException("Invalid IfcSurfaceOfRevolution", ifcSurfaceOfRevolution);
				else
					return revolutedSurface;
			}

			Handle(Geom_SurfaceOfLinearExtrusion) SurfaceFactory::BuildSurfaceOfLinearExtrusion(IIfcSurfaceOfLinearExtrusion^ ifcSurfaceOfLinearExtrusion)
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

				Handle(Geom_SurfaceOfLinearExtrusion) surface = EXEC_NATIVE->BuildSurfaceOfLinearExtrusion(sweptEdge, extrude);
				if (surface.IsNull())
					throw RaiseGeometryFactoryException("Surface of IfcSurfaceOfLinearExtrusion is invalid", ifcSurfaceOfLinearExtrusion);
				return surface;
			}

			TopoDS_Face SurfaceFactory::BuildCurveBoundedPlane(IIfcCurveBoundedPlane^ ifcCurveBoundedPlane)
			{
				Handle(Geom_Plane) basisPlane = BuildPlane(ifcCurveBoundedPlane->BasisSurface); //throws an exception with any failure
				TopoDS_Wire outerBoundary = WIRE_FACTORY->BuildWire(ifcCurveBoundedPlane->OuterBoundary, true);//throws an exception with any failure
				BRepBuilderAPI_MakeFace  faceMaker(basisPlane, outerBoundary);

				for each (IIfcCurve ^ innerCurve in ifcCurveBoundedPlane->InnerBoundaries)
				{
					TopoDS_Wire innerBound = WIRE_FACTORY->BuildWire(innerCurve, false);//throws an exception with any failure
					faceMaker.Add(innerBound);
				}

				if (faceMaker.IsDone())
				{
					gp_Trsf trsf;
					trsf.SetTransformation(basisPlane->Position(), gp::XOY());
					TopoDS_Face face = faceMaker.Face();
					face.Move(trsf);
					ShapeFix_Edge sfe;
					for (TopExp_Explorer exp(faceMaker.Face(), TopAbs_EDGE); exp.More(); exp.Next())
					{
						sfe.FixAddPCurve(TopoDS::Edge(exp.Current()), faceMaker.Face(), Standard_False);
					}
					return face;
				}
				else
					throw RaiseGeometryFactoryException("Invalid curve bounded plane", ifcCurveBoundedPlane);

			}
			TopoDS_Face SurfaceFactory::BuildCurveBoundedSurface(IIfcCurveBoundedSurface^ ifcCurveBoundedSurface)
			{
				throw gcnew NotImplementedException();
			}

			Handle(Geom_RectangularTrimmedSurface) SurfaceFactory::BuildRectangularTrimmedSurface(IIfcRectangularTrimmedSurface^ ifcRectangularTrimmedSurface)
			{
				XSurfaceType surfaceType;
				Handle(Geom_Surface) basisSurface = BuildSurface(ifcRectangularTrimmedSurface->BasisSurface, surfaceType); //throws an exception with any failure

				Handle(Geom_RectangularTrimmedSurface) geomTrim = new  Geom_RectangularTrimmedSurface(basisSurface, ifcRectangularTrimmedSurface->U1,
					ifcRectangularTrimmedSurface->U2, ifcRectangularTrimmedSurface->V1, ifcRectangularTrimmedSurface->V2);
				return geomTrim;
			}

			Handle(Geom_BSplineSurface) SurfaceFactory::BuildBSplineSurfaceWithKnots(IIfcBSplineSurfaceWithKnots^ ifcBSplineSurfaceWithKnots)
			{
				auto ifcControlPoints = ifcBSplineSurfaceWithKnots->ControlPoints;
				if (ifcControlPoints->Count < 2)
					throw RaiseGeometryFactoryException("Incorrect number of poles for Bspline surface, it must be at least 2", ifcBSplineSurfaceWithKnots);

				TColgp_Array2OfPnt poles(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->UUpper + 1, 1, (Standard_Integer)ifcBSplineSurfaceWithKnots->VUpper + 1);

				for (int u = 0; u <= ifcBSplineSurfaceWithKnots->UUpper; u++)
				{
					auto uRow = ifcControlPoints[u];
					for (int v = 0; v <= ifcBSplineSurfaceWithKnots->VUpper; v++)
					{
						poles.SetValue(u + 1, v + 1, gp_Pnt(uRow[v].X, uRow[v].Y, uRow[v].Z));
					}
				}

				TColStd_Array1OfReal uknots(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfReal vknots(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotVUpper);
				TColStd_Array1OfInteger uMultiplicities(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfInteger vMultiplicities(1, (Standard_Integer)ifcBSplineSurfaceWithKnots->KnotVUpper);
				int i = 1;
				for each (double knot in ifcBSplineSurfaceWithKnots->UKnots)
				{
					uknots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (double knot in ifcBSplineSurfaceWithKnots->VKnots)
				{
					vknots.SetValue(i, knot);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcBSplineSurfaceWithKnots->UMultiplicities)
				{
					uMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcBSplineSurfaceWithKnots->VMultiplicities)
				{
					vMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				Handle(Geom_BSplineSurface) hSurface = new Geom_BSplineSurface(poles, uknots, vknots, uMultiplicities, vMultiplicities, (Standard_Integer)ifcBSplineSurfaceWithKnots->UDegree, (Standard_Integer)ifcBSplineSurfaceWithKnots->VDegree);
				return hSurface;
			}

			Handle(Geom_BSplineSurface) SurfaceFactory::BuildRationalBSplineSurfaceWithKnots(IIfcRationalBSplineSurfaceWithKnots^ ifcRationalBSplineSurfaceWithKnots)
			{
				auto ifcControlPoints = ifcRationalBSplineSurfaceWithKnots->ControlPoints;
				if (ifcControlPoints->Count < 2)
					throw RaiseGeometryFactoryException("Incorrect number of poles for Bspline surface, it must be at least 2", ifcRationalBSplineSurfaceWithKnots);

				TColgp_Array2OfPnt poles(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->UUpper + 1, 1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->VUpper + 1);

				for (int u = 0; u <= ifcRationalBSplineSurfaceWithKnots->UUpper; u++)
				{
					auto uRow = ifcControlPoints[u];
					for (int v = 0; v <= ifcRationalBSplineSurfaceWithKnots->VUpper; v++)
					{
						poles.SetValue(u + 1, v + 1, gp_Pnt(uRow[v].X, uRow[v].Y, uRow[v].Z));
					}
				}
				auto ifcWeights = ifcRationalBSplineSurfaceWithKnots->Weights;
				TColStd_Array2OfReal weights(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->UUpper + 1, 1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->VUpper + 1);
				for (int u = 0; u <= ifcRationalBSplineSurfaceWithKnots->UUpper; u++)
				{
					List<Ifc4::MeasureResource::IfcReal>^ uRow = ifcWeights[u];
					for (int v = 0; v <= ifcRationalBSplineSurfaceWithKnots->VUpper; v++)
					{
						double r = uRow[v];
						weights.SetValue(u + 1, v + 1, r);
					}
				}


				TColStd_Array1OfReal uknots(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfReal vknots(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotVUpper);
				TColStd_Array1OfInteger uMultiplicities(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotUUpper);
				TColStd_Array1OfInteger vMultiplicities(1, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->KnotVUpper);
				int i = 1;
				for each (double knot in ifcRationalBSplineSurfaceWithKnots->UKnots)
				{
					uknots.SetValue(i, knot);
					i++;
				}
				i = 1;
				for each (double knot in ifcRationalBSplineSurfaceWithKnots->VKnots)
				{
					vknots.SetValue(i, knot);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcRationalBSplineSurfaceWithKnots->UMultiplicities)
				{
					uMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				i = 1;
				for each (int multiplicity in ifcRationalBSplineSurfaceWithKnots->VMultiplicities)
				{
					vMultiplicities.SetValue(i, multiplicity);
					i++;
				}

				Handle(Geom_BSplineSurface) hSurface = new Geom_BSplineSurface(poles, weights, uknots, vknots, uMultiplicities, vMultiplicities, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->UDegree, (Standard_Integer)ifcRationalBSplineSurfaceWithKnots->VDegree);
				return hSurface;
			}
			Handle(Geom_CylindricalSurface) SurfaceFactory::BuildCylindricalSurface(IIfcCylindricalSurface^ ifcCylindricalSurface)
			{
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcCylindricalSurface->Position, ax2))
					throw RaiseGeometryFactoryException("Invalid axis for surface", ifcCylindricalSurface);
				gp_Ax3 ax3(ax2);
				return new Geom_CylindricalSurface(ax3, ifcCylindricalSurface->Radius);
			}

#pragma endregion
		}
	}
}
