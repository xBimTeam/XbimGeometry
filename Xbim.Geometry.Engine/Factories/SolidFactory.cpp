#include "SolidFactory.h"
#include "WireFactory.h"
#include "ProfileFactory.h"
#include "ShellFactory.h"
#include "SurfaceFactory.h"
#include "GeometryFactory.h"
#include "EdgeFactory.h"
#include "BooleanFactory.h"
#include <gp_Ax2.hxx>
#include <BRepCheck_Shell.hxx>
#include <ShapeFix_Shell.hxx>
#include "../BRep/XCompound.h"
#include "../BRep/XSolid.h"
#include "../BRep/XShell.h"
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <TopoDS.hxx>
#include <BRepLib.hxx>
#include "../BRep/XShape.h"
#include "../BRep/XVertex.h"
#include "../BRep/XEdge.h"
#include "../BRep/XWire.h"
#include "../BRep/XFace.h"
#include "CheckClosedStatus.h"
#include "../XbimSolid.h"
#include <GeomLib.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <ShapeFix_Solid.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepBuilderAPI_Transform.hxx>

using namespace System;
using namespace Xbim::Geometry::BRep;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			bool SolidFactory::TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape)
			{
				return EXEC_NATIVE->TryUpgrade(solid, shape);
			}

			IXShape^ SolidFactory::Convert(System::String^ brepStr)
			{
				return SHAPE_FACTORY->Convert(brepStr);
			}


			IXShape^ SolidFactory::Build(IIfcFacetedBrep^ ifcBrep)
			{
				TopoDS_Shape shape = BuildFacetedBrep(ifcBrep);
				return  ShapeFactory::GetXbimShape(shape);
			}

			IXShape^ SolidFactory::Build(IIfcFaceBasedSurfaceModel^ ifcSurfaceModel)
			{
				TopoDS_Shape shape = BuildFaceBasedSurfaceModel(ifcSurfaceModel);
				return  ShapeFactory::GetXbimShape(shape);
			}

			IXSolid^ SolidFactory::Build(IIfcCsgPrimitive3D^ ifcCsgPrimitive)
			{
				TopoDS_Solid topoSolid = BuildCsgPrimitive3D(ifcCsgPrimitive);
				if (topoSolid.IsNull())
					throw RaiseGeometryFactoryException("Failure building IfcCsgPrimitive3D", ifcCsgPrimitive);
				return gcnew XSolid(topoSolid);
			}

			IXSolid^ SolidFactory::Build(IIfcHalfSpaceSolid^ ifcHalfSpaceSolid)
			{
				auto halfSpaceSolid = BuildHalfSpace(ifcHalfSpaceSolid);
				if (halfSpaceSolid.IsNull() || halfSpaceSolid.ShapeType() != TopAbs_SOLID) return nullptr;
				else
					return gcnew XSolid(halfSpaceSolid);
			}

			IXShape^ SolidFactory::Build(IIfcShellBasedSurfaceModel^ ifcSurfaceModel)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ SolidFactory::Build(IIfcTessellatedItem^ ifcTessellatedItem)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			IXShape^ SolidFactory::Build(IIfcSectionedSpine^ ifcSectionedSpine)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			TopoDS_Solid SolidFactory::BuildHalfSpace(IIfcHalfSpaceSolid^ ifcHalfSpaceSolid)
			{
				//if (dynamic_cast<IIfcBoxedHalfSpace^>(ifcHalfSpaceSolid)) SRL treat as normal half space solid, the boxed bit is only for computational efficiency		

				auto elementarySurface = dynamic_cast<IIfcElementarySurface^>(ifcHalfSpaceSolid->BaseSurface);
				if (elementarySurface == nullptr)
					throw RaiseGeometryFactoryException("Informal proposition. Only IfcElementarySurfaces are supported for IfcHalfSpaceSolids", ifcHalfSpaceSolid);
				XSurfaceType surfaceType;
				Handle(Geom_Surface) baseSurface = SURFACE_FACTORY->BuildSurface(ifcHalfSpaceSolid->BaseSurface, surfaceType);

				auto planarSurface = dynamic_cast<IIfcPlane^>(elementarySurface);
				auto cylindricalSurface = dynamic_cast<IIfcCylindricalSurface^>(elementarySurface);
				auto sphericalSurface = dynamic_cast<IIfcSphericalSurface^>(elementarySurface);
				auto toroidalSurface = dynamic_cast<IIfcToroidalSurface^>(elementarySurface);
				auto polygonalBoundedHalfSpace = dynamic_cast<IIfcPolygonalBoundedHalfSpace^>(ifcHalfSpaceSolid);

				TopoDS_Face face;
				gp_Pnt pointInMaterial;
				if (planarSurface != nullptr)
				{
					Handle(Geom_Plane) plane = SURFACE_FACTORY->BuildPlane(planarSurface, true);
					gp_Vec normalDir = plane->Axis().Direction();
					if (ifcHalfSpaceSolid->AgreementFlag) normalDir.Reverse();
					pointInMaterial = plane->Location().Translated(normalDir * _modelService->OneMeter);
					
					BRep_Builder b;
					b.MakeFace(face, plane, _modelService->Precision);
				}
				else if (cylindricalSurface != nullptr)
				{
					auto surface = SURFACE_FACTORY->BuildCylindricalSurface(cylindricalSurface);
					gp_Dir normalDir = surface->Axis().Direction(); //oriented towards the outside of the cylinder
					pointInMaterial = surface->Location(); //location of cylinder is always in the material
					if (ifcHalfSpaceSolid->AgreementFlag) //the material is outside the cylinder
					{
						normalDir.Reverse();
						gp_Vec displace = normalDir;
						displace *= cylindricalSurface->Radius * 2;
						pointInMaterial = surface->Location().Translated(displace);
					}

					face = BRepBuilderAPI_MakeFace(surface, _modelService->Precision);

				}
				else if (sphericalSurface != nullptr)
				{
					auto surface = SURFACE_FACTORY->BuildSphericalSurface(sphericalSurface);
					gp_Dir normalDir = surface->Axis().Direction(); //oriented away from the centre
					pointInMaterial = surface->Location(); //location of sphere is always in the material
					if (ifcHalfSpaceSolid->AgreementFlag)
					{
						normalDir.Reverse();
						gp_Vec displace = normalDir;
						displace *= sphericalSurface->Radius * 2;
						pointInMaterial = surface->Location().Translated(displace);
					}
					face = BRepBuilderAPI_MakeFace(surface, _modelService->Precision);
				}
				else if (toroidalSurface != nullptr)
				{
					//SRL further work is needed here but no evidence of toroidal surfaces has been found in ifc files yet, so prioritising other actions
					throw RaiseGeometryFactoryException("IfcToroidalSurface for HalfSpace is not currently supported", ifcHalfSpaceSolid);
				}
				BRepPrimAPI_MakeHalfSpace hasMaker(face, pointInMaterial);
				if (!hasMaker.IsDone())
					throw RaiseGeometryFactoryException("Failure building IfcHalfSpaceSolid", ifcHalfSpaceSolid);
				auto halfSpace = hasMaker.Solid();
				if (polygonalBoundedHalfSpace != nullptr)
				{
					if (2 != (int)polygonalBoundedHalfSpace->PolygonalBoundary->Dim)
						throw RaiseGeometryFactoryException("IfcPolygonalBoundedHalfSpace must have a 2D polygonal boundary", ifcHalfSpaceSolid);
					if (planarSurface == nullptr)
						throw RaiseGeometryFactoryException("IfcPolygonalBoundedHalfSpace must have a planar surface", ifcHalfSpaceSolid);
					auto wire = WIRE_FACTORY->BuildWire(polygonalBoundedHalfSpace->PolygonalBoundary, false); //it will have a 2d bound
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IfcPolygonalBoundedHalfSpace polygonal boundary could not be built", ifcHalfSpaceSolid);
					
					
					bool isCounterClockwise;
					wire = PROFILE_FACTORY->EXEC_NATIVE->MakeValidAreaProfileWire(wire, isCounterClockwise);
					if (wire.IsNull())
					{
						LogDebug(polygonalBoundedHalfSpace, "Empty IfcPolygonBoundedHalfSpaceSolid has been ignored");
						return TopoDS_Solid();
					}
					if (!isCounterClockwise) wire.Reverse();
					
					auto polyBoundFace = PROFILE_FACTORY->EXEC_NATIVE->MakeFace(wire);
					auto str = (gcnew XFace(polyBoundFace))->BrepString();
					TopLoc_Location location;
					if (polygonalBoundedHalfSpace->Position != nullptr)
						location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(polygonalBoundedHalfSpace->Position);
					double shiftDistance = 200 * ModelGeometryService->OneMeter;//seems big enough for most models
					TopoDS_Solid substractionBody = EXEC_NATIVE->MakeSweptSolid(polyBoundFace, gp_Vec(0, 0, shiftDistance));
					ShapeFix_Shape shapeFixer(substractionBody);
					bool fixed = shapeFixer.Perform();
					if (fixed && shapeFixer.Shape().ShapeType()==TopAbs_SOLID)
						substractionBody = TopoDS::Solid(shapeFixer.Shape());
					gp_Trsf shift;
					shift.SetTranslation(gp_Vec(0, 0, -shiftDistance / 2));
					substractionBody.Move(shift);
					substractionBody.Move(location);
					bool hasWarnings;
					
					TopoDS_Shape shape = BOOLEAN_FACTORY->EXEC_NATIVE->Intersect(halfSpace, substractionBody, ModelGeometryService->Precision, hasWarnings);				
					if (hasWarnings)
						LogDebug(polygonalBoundedHalfSpace, "Warnings generated building half space solid. See logs");
					halfSpace = EXEC_NATIVE->CastToSolid(shape); //not a solid, then normally empty compound
					if (halfSpace.IsNull())
						LogDebug(polygonalBoundedHalfSpace, "Empty IfcPolygonBoundedHalfSpaceSolid has been ignored");
					else
						return halfSpace;
				}
				return halfSpace;
			}


			IXShape^ SolidFactory::Build(IIfcSolidModel^ ifcSolid)
			{
				TopoDS_Shape topoShape = BuildSolidModel(ifcSolid);
				if (topoShape.IsNull())
					throw RaiseGeometryFactoryException("Failure building IIfcSolidModel", ifcSolid);
				topoShape.Closed(true);
				return  ShapeFactory::GetXbimShape(topoShape);
			}
			

			///this method builds all solid models and is the main entry point
			//all methods called will throw an excpetion if they cannot build their part of a solid
			TopoDS_Shape SolidFactory::BuildSolidModel(IIfcSolidModel^ ifcSolid)
			{
				XSolidModelType solidModelType;
				if (!Enum::TryParse<XSolidModelType>(ifcSolid->ExpressType->ExpressName, solidModelType))
					throw RaiseGeometryFactoryException("Unsupported solidmodel type", ifcSolid);

				switch (solidModelType)
				{
					case XSolidModelType::IfcFacetedBrep:
						return BuildFacetedBrep(static_cast<IIfcFacetedBrep^>(ifcSolid));
					case XSolidModelType::IfcSweptDiskSolid:
						return BuildSweptDiskSolid(static_cast<IIfcSweptDiskSolid^>(ifcSolid));
					case XSolidModelType::IfcExtrudedAreaSolid:
						return BuildExtrudedAreaSolid(static_cast<IIfcExtrudedAreaSolid^>(ifcSolid));
					case XSolidModelType::IfcSweptDiskSolidPolygonal:
						return BuildSweptDiskSolidPolygonal(static_cast<IIfcSweptDiskSolidPolygonal^>(ifcSolid));
					case XSolidModelType::IfcSurfaceCurveSweptAreaSolid:
						return BuildSurfaceCurveSweptAreaSolid(static_cast<IIfcSurfaceCurveSweptAreaSolid^>(ifcSolid));
					case XSolidModelType::IfcCsgSolid:
						return BuildCsgSolid(static_cast<IIfcCsgSolid^>(ifcSolid));
					case XSolidModelType::IfcExtrudedAreaSolidTapered:
						return BuildExtrudedAreaSolidTapered(static_cast<IIfcExtrudedAreaSolidTapered^>(ifcSolid));
					case XSolidModelType::IfcAdvancedBrep:
						return BuildAdvancedBrep(static_cast<IIfcAdvancedBrep^>(ifcSolid));
					case XSolidModelType::IfcSectionedSolidHorizontal:
						return BuildSectionedSolidHorizontal(static_cast<IfcSectionedSolidHorizontal^>(ifcSolid));
					case XSolidModelType::IfcDirectrixDerivedReferenceSweptAreaSolid:
						return BuildDirectrixDerivedReferenceSweptAreaSolid(static_cast<IfcDirectrixDerivedReferenceSweptAreaSolid^>(ifcSolid));

					//TODO: SRL: the following methods will need to be implemented as Version 6, defaulting to version 5 implementation	
					case XSolidModelType::IfcAdvancedBrepWithVoids:
						return gcnew XbimSolid(static_cast<IIfcAdvancedBrepWithVoids^>(ifcSolid), Logger(), _modelService);
					case XSolidModelType::IfcFacetedBrepWithVoids:
						return gcnew XbimSolid(static_cast<IIfcFacetedBrepWithVoids^>(ifcSolid), Logger(), _modelService);
					case XSolidModelType::IfcFixedReferenceSweptAreaSolid:
						return gcnew XbimSolid(static_cast<IIfcFixedReferenceSweptAreaSolid^>(ifcSolid), Logger(), _modelService);
					case XSolidModelType::IfcRevolvedAreaSolid:
						return gcnew XbimSolid(static_cast<IIfcRevolvedAreaSolid^>(ifcSolid), Logger(), _modelService);
					case XSolidModelType::IfcRevolvedAreaSolidTapered:
						return gcnew XbimSolid(static_cast<IIfcRevolvedAreaSolidTapered^>(ifcSolid), Logger(), _modelService);

					default:
						break;
				}
				throw RaiseGeometryFactoryException("Not implemented. SolidModel type", ifcSolid);
			}



			TopoDS_Shape SolidFactory::BuildDirectrixDerivedReferenceSweptAreaSolid(IfcDirectrixDerivedReferenceSweptAreaSolid^ directrixDerivedReferenceSweptAreaSolid)
			{
				Handle(Geom_Curve) spine;
				auto sweptArea = PROFILE_FACTORY->BuildProfileFace(directrixDerivedReferenceSweptAreaSolid->SweptArea);
				
				Ifc4::Interfaces::IIfcLengthMeasure^ startLen = dynamic_cast<Ifc4::Interfaces::IIfcLengthMeasure^>(directrixDerivedReferenceSweptAreaSolid->StartParam);
				Ifc4::Interfaces::IIfcLengthMeasure^ endLen = dynamic_cast<Ifc4::Interfaces::IIfcLengthMeasure^>(directrixDerivedReferenceSweptAreaSolid->EndParam);
				Ifc4x3::MeasureResource::IfcParameterValue^ startParam = dynamic_cast<Ifc4x3::MeasureResource::IfcParameterValue^>(directrixDerivedReferenceSweptAreaSolid->StartParam);
				Ifc4x3::MeasureResource::IfcParameterValue^ endParam = dynamic_cast<Ifc4x3::MeasureResource::IfcParameterValue^>(directrixDerivedReferenceSweptAreaSolid->EndParam);
				double firstParam;
				double lastParam;

				if (startLen && endLen)
				{
					spine = CURVE_FACTORY->BuildCurve(directrixDerivedReferenceSweptAreaSolid->Directrix);
					firstParam = static_cast<double>(startLen->Value);
					lastParam = static_cast<double>(endLen->Value);
				}
				else if (startParam && endParam) {
					spine = CURVE_FACTORY->BuildCurve(directrixDerivedReferenceSweptAreaSolid->Directrix);
					double length = spine->LastParameter() - spine->FirstParameter();
					firstParam = static_cast<double>(startParam->Value) * length;
					lastParam = static_cast<double>(endParam->Value) * length;
				}
				
				IIfcDirection^ xdir = directrixDerivedReferenceSweptAreaSolid->FixedReference;
				std::vector<TopoDS_Wire> sections;
				auto step = 100 * _modelService->OneMillimeter;
				int n = (int)((lastParam - firstParam) / step);
				auto segmentedRef = Handle(Geom_SegmentedReferenceCurve)::DownCast(spine);
				
				for (int i = 0; i <= n; i++)
				{
					gp_Pnt loc;
					gp_Vec normal;
					double len = firstParam + (i * step);
					spine->D1(len, loc, normal);
					gp_Vec xVec(xdir->X, xdir->Y, xdir->Z);

					if (segmentedRef)
					{
						auto superElevationAndTilt = segmentedRef->GetSuperelevationAndCantTiltAt(len);
						Standard_Real tilt = std::get<1>(superElevationAndTilt);
						gp_Ax1 rotationAxis(gp_Pnt(0.0, 0.0, 0.0), normal);
						xVec.Rotate(rotationAxis, tilt);
					}

					auto moved = MovedSection(loc, normal, xVec, sweptArea);
					TopoDS_Wire section = BRepTools::OuterWire(moved);
					sections.push_back(section);
				}

				auto trimmed = new Geom_TrimmedCurve(spine, firstParam, lastParam);
				Handle(Geom_BSplineCurve) bSpline = CURVE_FACTORY->ToBSpline(trimmed, n);
				auto solid = EXEC_NATIVE->BuildExtrudedAreaSolid(bSpline, sections, ModelGeometryService->Precision);
				if (directrixDerivedReferenceSweptAreaSolid->Position != nullptr) {
					TopLoc_Location location;
					if (GEOMETRY_FACTORY->ToLocation(directrixDerivedReferenceSweptAreaSolid->Position, location)) {
						solid.Move(location);
					}
				}

				return solid;
			}

			TopoDS_Shape SolidFactory::BuildSectionedSolidHorizontal(IfcSectionedSolidHorizontal^ sectionedSolidHorizontal)
			{
				if (sectionedSolidHorizontal->CrossSectionPositions->Count != sectionedSolidHorizontal->CrossSections->Count)
					throw RaiseGeometryFactoryException("Number of cross-section position doesn't match the number of cross-sections", sectionedSolidHorizontal);

				Handle(Geom_Curve) spine = CURVE_FACTORY->BuildCurve(sectionedSolidHorizontal->Directrix);

				std::vector<TopoDS_Wire> sections;
				size_t lastEdgeCount = 0;
				std::vector<Standard_Real> distancesAlong;

				// It is assumed that all sections will be of the same type
				bool isIfcParameterizedProfileDef = dynamic_cast<IIfcParameterizedProfileDef^>(sectionedSolidHorizontal->CrossSections[0]) != nullptr;
			
				for (int i = 0; i < sectionedSolidHorizontal->CrossSections->Count; i++)
				{
					/*XProfileDefType profileType;
					TopoDS_Wire outerWire = PROFILE_FACTORY->BuildProfileWire(sectionedSolidHorizontal->CrossSections[i], profileType);*/
					TopoDS_Face sectionFace = PROFILE_FACTORY->BuildProfileFace(sectionedSolidHorizontal->CrossSections[i]);

					gp_Pnt loc;
					gp_Vec normal;
					gp_Vec refVec;
					gp_Vec up;
					bool hasAxis;
					bool hasRef;

					// The profile placement is not necessarily derived 
					// from the spine curve if the Axis and RefDirection is specified
					IfcAxis2PlacementLinear^ placement = sectionedSolidHorizontal->CrossSectionPositions[i];
					if (placement->Axis != nullptr)
					{
						if (!GEOMETRY_FACTORY->BuildDirection3d(placement->Axis, up))
							throw RaiseGeometryFactoryException("IfcAxis2PlacementLinear Axis Direction is invalid ", placement);
						up.Normalize();
						hasAxis = true;
					}

					if (placement->RefDirection != nullptr)
					{
						if (!GEOMETRY_FACTORY->BuildDirection3d(placement->RefDirection, refVec))
							throw RaiseGeometryFactoryException("IfcAxis2PlacementLinear Reference Direction is invalid ", placement);
						refVec.Normalize();
						hasRef = true;
					}
					Standard_Real distanceAlong;
					
					if (hasAxis && hasRef) {
						auto distanceAlongPoint = static_cast<IfcPointByDistanceExpression^>(placement->Location);
						distanceAlong = GEOMETRY_FACTORY->GetDistanceAlong(distanceAlongPoint);
						normal = refVec.Crossed(up);
					}
					else 
					{
						GEOMETRY_FACTORY->GetTangentAtPlacement(placement, loc, normal, up, distanceAlong);
						refVec = up.Crossed(normal);
					}
					distancesAlong.push_back(distanceAlong);
					
					// curve tangent is the section normal
					TopoDS_Wire section = MovedSection(loc, normal, refVec, BRepTools::OuterWire(sectionFace));
					sections.push_back(section);

					// Should have similar edge count if not IfcParameterizedProfileDef
					if (!isIfcParameterizedProfileDef)
					{
						std::vector<TopoDS_Edge> edges;
						for (TopExp_Explorer explorer(section, TopAbs_EDGE); explorer.More(); explorer.Next())
						{
							TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
							edges.push_back(edge);

						}

						if (i > 0 && edges.size() != lastEdgeCount)
							throw RaiseGeometryFactoryException("Cross-sections should have the same number of points and edges", sectionedSolidHorizontal);

						lastEdgeCount = edges.size();
					}
				}

				
				Handle(Geom_ConvertibleToBSpline) convertibleToBSpline = Handle(Geom_ConvertibleToBSpline)::DownCast(spine);

				if (convertibleToBSpline)
				{
					int n = (int)std::clamp((distancesAlong.back() - distancesAlong.front()) / _modelService->OneMeter, 10., 1000.);
					auto trimmed = convertibleToBSpline->ToBSpline(distancesAlong.front(), distancesAlong.back(), n);
					return EXEC_NATIVE->BuildSectionedSolid(trimmed, sections);
				}

				Handle(Geom_Curve) trimmed = CURVE_FACTORY->TrimCurveByAtDistances(spine, distancesAlong.front(), distancesAlong.back());
				return EXEC_NATIVE->BuildSectionedSolid(trimmed, sections);
			}

			
			TopoDS_Shape SolidFactory::BuildSurfaceCurveSweptAreaSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSurfaceCurveSweptAreaSolid)
			{
				auto compositeProfile = dynamic_cast<IIfcCompositeProfileDef^>(ifcSurfaceCurveSweptAreaSolid->SweptArea);
				if (compositeProfile != nullptr)
				{
					TopoDS_Compound shape;
					BRep_Builder b;
					b.MakeCompound(shape);
					for each (auto profileDef in compositeProfile->Profiles)
					{ 
						auto solid = BuildSurfaceCurveSweptAreaSolid(ifcSurfaceCurveSweptAreaSolid, profileDef);
						b.Add(shape, solid);
					}
					return shape;
				}
				else 
					return BuildSurfaceCurveSweptAreaSolid(ifcSurfaceCurveSweptAreaSolid, ifcSurfaceCurveSweptAreaSolid->SweptArea);
			}
			
			/// <summary>
			/// Private method that allows the profil def to be overriden to support building composite profile definitions
			/// </summary>
			/// <param name="ifcSurfaceCurveSweptAreaSolid"></param>
			/// <param name="profileDef"></param>
			/// <returns></returns>
			TopoDS_Solid SolidFactory::BuildSurfaceCurveSweptAreaSolid(IIfcSurfaceCurveSweptAreaSolid^ ifcSurfaceCurveSweptAreaSolid, IIfcProfileDef^ profileDef)
			{
				if (profileDef == nullptr) profileDef = ifcSurfaceCurveSweptAreaSolid->SweptArea; //this is a work around to allow backward compatibility with V5 to allow V5 to use the new code
				//build the swept area
				if (profileDef->ProfileType != IfcProfileTypeEnum::AREA)
					throw RaiseGeometryFactoryException("Rule SweptAreaType must be IfcProfileTypeEnum::AREA", profileDef);
				auto sweptArea = PROFILE_FACTORY->BuildProfileFace(profileDef);
				if (sweptArea.IsNull())
					throw RaiseGeometryFactoryException("Error build swept area", profileDef);

				//build the reference surface
				XSurfaceType surfaceType;
				auto refSurface = SURFACE_FACTORY->BuildSurface(ifcSurfaceCurveSweptAreaSolid->ReferenceSurface, surfaceType);
				if (refSurface.IsNull())
					throw RaiseGeometryFactoryException("Reference Surface is invalid", ifcSurfaceCurveSweptAreaSolid->ReferenceSurface);
				bool isPlanarReferenceSurface = (surfaceType == XSurfaceType::IfcPlane);

				//build the directrix
				auto directrixWire = WIRE_FACTORY->BuildDirectrixWire(ifcSurfaceCurveSweptAreaSolid->Directrix, NULLABLE_TO_DOUBLE(ifcSurfaceCurveSweptAreaSolid->StartParam), NULLABLE_TO_DOUBLE(ifcSurfaceCurveSweptAreaSolid->EndParam));
				if (directrixWire.IsNull())
					throw RaiseGeometryFactoryException("Directrix is invalid", ifcSurfaceCurveSweptAreaSolid->Directrix);
				
				auto solid = EXEC_NATIVE->BuildSurfaceCurveSweptAreaSolid(sweptArea, refSurface, directrixWire, isPlanarReferenceSurface, ModelGeometryService->MinimumGap);
				if(solid.IsNull())
					throw RaiseGeometryFactoryException("Failure building SurfaceCurveSweptAreaSolid", ifcSurfaceCurveSweptAreaSolid);
				TopLoc_Location location;
				if (ifcSurfaceCurveSweptAreaSolid->Position != nullptr) location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(ifcSurfaceCurveSweptAreaSolid->Position);
				if (!location.IsIdentity()) solid.Move(location, false);
				return solid;
			}


			TopoDS_Solid SolidFactory::BuildSweptDiskSolidPolygonal(IIfcSweptDiskSolidPolygonal^ ifcSweptDiskSolidPolygonal)
			{
				if (ifcSweptDiskSolidPolygonal->Radius <= 0)
					throw RaiseGeometryFactoryException("Radius must be greater than 0", ifcSweptDiskSolidPolygonal);
				if (ifcSweptDiskSolidPolygonal->InnerRadius.HasValue && ifcSweptDiskSolidPolygonal->InnerRadius.Value >= ifcSweptDiskSolidPolygonal->Radius)
					throw RaiseGeometryFactoryException("Inner radius is greater than outer radius", ifcSweptDiskSolidPolygonal);

				auto directrix = WIRE_FACTORY->BuildDirectrixWire(ifcSweptDiskSolidPolygonal->Directrix, NULLABLE_TO_DOUBLE(ifcSweptDiskSolidPolygonal->StartParam), NULLABLE_TO_DOUBLE(ifcSweptDiskSolidPolygonal->EndParam));
				
				if (directrix.IsNull())
					throw RaiseGeometryFactoryException("Could not build directrix", ifcSweptDiskSolidPolygonal);

				double filletRadius = NULLABLE_TO_DOUBLE(ifcSweptDiskSolidPolygonal->FilletRadius);
				if (!double::IsNaN(filletRadius))
				{
					TopoDS_Wire filletedDirectrix;
					if (WIRE_FACTORY->Fillet(directrix, filletedDirectrix, filletRadius))
						directrix = filletedDirectrix;
					else
						LogWarning(ifcSweptDiskSolidPolygonal, "Error building directix with fillets");
				}
				double innerRadius = NULLABLE_TO_DOUBLE(ifcSweptDiskSolidPolygonal->InnerRadius);

				TopoDS_Solid solid = EXEC_NATIVE->BuildSweptDiskSolid(directrix, ifcSweptDiskSolidPolygonal->Radius, innerRadius);
				return solid;
			}

			TopoDS_Solid SolidFactory::BuildSweptDiskSolid(IIfcSweptDiskSolid^ ifcSweptDiskSolid)
			{
				auto ifcSweptDiskSolidPolygonal = dynamic_cast<IIfcSweptDiskSolidPolygonal^>(ifcSweptDiskSolid);
				if (ifcSweptDiskSolidPolygonal != nullptr)
					return BuildSweptDiskSolidPolygonal(ifcSweptDiskSolidPolygonal);
				else
				{
					if (ifcSweptDiskSolid->Radius <= 0)
						throw RaiseGeometryFactoryException("Radius must be greater than 0", ifcSweptDiskSolid);
					if (ifcSweptDiskSolid->InnerRadius.HasValue && ifcSweptDiskSolid->InnerRadius.Value >= ifcSweptDiskSolid->Radius)
						throw RaiseGeometryFactoryException("Inner radius is greater than outer radius", ifcSweptDiskSolid);

					auto directrix = WIRE_FACTORY->BuildDirectrixWire(ifcSweptDiskSolid->Directrix, NULLABLE_TO_DOUBLE(ifcSweptDiskSolid->StartParam), NULLABLE_TO_DOUBLE(ifcSweptDiskSolid->EndParam));


					if (directrix.IsNull())
						throw RaiseGeometryFactoryException("Could not build directrix", ifcSweptDiskSolid);


					double innerRadius = NULLABLE_TO_DOUBLE(ifcSweptDiskSolid->InnerRadius);

					TopoDS_Solid solid = EXEC_NATIVE->BuildSweptDiskSolid(directrix, ifcSweptDiskSolid->Radius, innerRadius);
					return solid;
				}

			}

			TopoDS_Shape SolidFactory::BuildExtrudedAreaSolid(IIfcExtrudedAreaSolid^ extrudedSolid)
			{
				auto compositeProfile = dynamic_cast<IIfcCompositeProfileDef^>(extrudedSolid->SweptArea);
				if (compositeProfile != nullptr)
				{
					TopoDS_Compound shape;
					BRep_Builder b;
					b.MakeCompound(shape);
					for each (auto profileDef in compositeProfile->Profiles)
					{
						auto solid = BuildExtrudedAreaSolid(extrudedSolid, profileDef);
						b.Add(shape, solid);
					}
					return shape;
				}
				else
					return BuildExtrudedAreaSolid(extrudedSolid, extrudedSolid->SweptArea);
			}


			TopoDS_Shape SolidFactory::BuildExtrudedAreaSolid(IIfcExtrudedAreaSolid^ extrudedSolid, IIfcProfileDef^ profileDef)
			{
				if (profileDef == nullptr) profileDef = extrudedSolid->SweptArea; //this is a work around to allow backward compatibility with V5 to allow V5 to use the new code
				if (extrudedSolid->Depth <= 0)
					throw RaiseGeometryFactoryException("Extruded Solid depth must be greater than 0", extrudedSolid);
				gp_Vec extrudeDirection;
				if (!GEOMETRY_FACTORY->BuildDirection3d(extrudedSolid->ExtrudedDirection, extrudeDirection))
					throw RaiseGeometryFactoryException("Extruded Solid sweep dirction is illegal", extrudedSolid->ExtrudedDirection);
				TopoDS_Face sweptArea = PROFILE_FACTORY->BuildProfileFace(profileDef); //if this fails it will throw an exception
				TopLoc_Location location;
				if (extrudedSolid->Position != nullptr)
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(extrudedSolid->Position);
				
				TopoDS_Solid solid = EXEC_NATIVE->BuildExtrudedAreaSolid(sweptArea, extrudeDirection, extrudedSolid->Depth, location);
				if (solid.IsNull() || solid.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Extruded Solid could not be built", extrudedSolid);

				return solid;
			}

			TopoDS_Shape SolidFactory::BuildExtrudedAreaSolidTapered(IIfcExtrudedAreaSolidTapered^ extrudedSolidTapered)
			{
				auto compositeProfile = dynamic_cast<IIfcCompositeProfileDef^>(extrudedSolidTapered->SweptArea);
				if (compositeProfile != nullptr)
				{
					auto endCompositeProfile = dynamic_cast<IIfcCompositeProfileDef^>(extrudedSolidTapered->EndSweptArea);
					if(endCompositeProfile == nullptr || endCompositeProfile->Profiles->Count != compositeProfile->Profiles->Count)
						throw RaiseGeometryFactoryException("Extruded Solid Tapered must having eqivalent Composite Profile definitions for start and end conditions", extrudedSolidTapered);
					TopoDS_Compound shape;
					BRep_Builder b;
					b.MakeCompound(shape);
					auto endCompositeProfileIter = endCompositeProfile->Profiles->GetEnumerator();
					for each (auto profileDef in compositeProfile->Profiles)
					{
						auto solid = BuildExtrudedAreaSolidTapered(extrudedSolidTapered, profileDef, endCompositeProfileIter->Current);
						b.Add(shape, solid);
						endCompositeProfileIter->MoveNext();
					}
					return shape;
				}
				else
					return BuildExtrudedAreaSolidTapered(extrudedSolidTapered, extrudedSolidTapered->SweptArea, extrudedSolidTapered->EndSweptArea);
			}

			TopoDS_Shape SolidFactory::BuildExtrudedAreaSolidTapered(IIfcExtrudedAreaSolidTapered^ extrudedSolidTapered, IIfcProfileDef^ startProfileDef, IIfcProfileDef^ endProfileDef)
			{
				if (startProfileDef == nullptr)
				{
					startProfileDef = extrudedSolidTapered->SweptArea; //this is a work around to allow backward compatibility with V5 to allow V5 to use the new code
					endProfileDef = extrudedSolidTapered->EndSweptArea;
				}
				if (extrudedSolidTapered->Depth <= 0)
					throw RaiseGeometryFactoryException("Extruded Solid depth must be greater than 0", extrudedSolidTapered);
				gp_Vec extrudeDirection;
				if (!GEOMETRY_FACTORY->BuildDirection3d(extrudedSolidTapered->ExtrudedDirection, extrudeDirection))
					throw RaiseGeometryFactoryException("Extruded Solid sweep direction is illegal", extrudedSolidTapered->ExtrudedDirection);
				TopoDS_Face sweptArea = PROFILE_FACTORY->BuildProfileFace(startProfileDef); //if this fails it will throw an exception
				TopoDS_Face endSweptArea = PROFILE_FACTORY->BuildProfileFace(endProfileDef); //if this fails it will throw an exception
				TopLoc_Location location;
				if (extrudedSolidTapered->Position != nullptr)
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(extrudedSolidTapered->Position);
				TopoDS_Shape solid = EXEC_NATIVE->BuildExtrudedAreaSolidTapered(sweptArea, endSweptArea, extrudeDirection, extrudedSolidTapered->Depth, location, ModelGeometryService->Precision);
				if (solid.IsNull() || solid.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Extruded Solid Tapered could not be built", extrudedSolidTapered);

				return solid;
			}
			

			TopoDS_Shape SolidFactory::BuildFacetedBrep(IIfcFacetedBrep^ facetedBrep)
			{
				bool isFixed;
				return SHELL_FACTORY->BuildClosedShell(facetedBrep->Outer, isFixed); //throws exeptions

			}

			/// <summary>
			/// FC4 CHANGE  The entity has been deprecated and shall not be used. The entity IfcFacetedBrep shall be used instead. Implemented for backward compatibility
			/// </summary>
			/// <param name="faceBasedSurfaceModel"></param>
			/// <returns></returns>
			TopoDS_Shape SolidFactory::BuildFaceBasedSurfaceModel(IIfcFaceBasedSurfaceModel^ faceBasedSurfaceModel)
			{
				TopoDS_Compound compound;
				BRep_Builder builder;
				builder.MakeCompound(compound);

				for each (IIfcConnectedFaceSet ^ faceSet in faceBasedSurfaceModel->FbsmFaces)
				{
					bool isFixed;
					TopoDS_Shape shape = SHELL_FACTORY->BuildConnectedFaceSet(faceSet, isFixed);
					builder.Add(compound, shape);
				}
				return compound;
			}

			TopoDS_Shape SolidFactory::BuildPolygonalFaceSet(IIfcPolygonalFaceSet^ ifcPolygonalFaceSet)
			{
				bool isFixed;
				TopoDS_Shape shape = SHELL_FACTORY->BuildPolygonalFaceSet(ifcPolygonalFaceSet, isFixed);
				if (ifcPolygonalFaceSet->Closed.HasValue && ifcPolygonalFaceSet->Closed.Value)
				{
					ShapeFix_Solid  sfs;
					if (shape.ShapeType() == TopAbs_COMPOUND)
					{
						//any shells in here will have been fixed
						BRep_Builder b;
						TopoDS_Compound solidCompound;
						b.MakeCompound(solidCompound);

						for (TopoDS_Iterator childIterator(shape); childIterator.More(); childIterator.Next())
							if (childIterator.Value().ShapeType() == TopAbs_SHELL)
								b.Add(solidCompound, sfs.SolidFromShell(TopoDS::Shell(childIterator.Value())));
						return solidCompound;
					}
					else if (shape.ShapeType() == TopAbs_SHELL)//it will be a shell
					{
						return sfs.SolidFromShell(TopoDS::Shell(shape));
					}
					else
						throw RaiseGeometryFactoryException("Failed to build closed polygonal face set", ifcPolygonalFaceSet);
				}
				else
					return shape;
			}

			TopoDS_Shape SolidFactory::BuildAdvancedBrep(IIfcAdvancedBrep^ ifcAdvancedBrep)
			{
				bool isFixed;
				TopoDS_Shape shape = SHELL_FACTORY->BuildClosedShell(ifcAdvancedBrep->Outer, isFixed);
				if (shape.IsNull() )
					throw RaiseGeometryFactoryException("Error creating solid from advanced brep", ifcAdvancedBrep);
				return shape;
			}

			TopoDS_Solid SolidFactory::BuildCsgSolid(IIfcCsgSolid^ ifcCsgSolid)
			{
				//at the root of a csg solid is either a boolean result or a csg solid primitive
				IIfcBooleanResult^ booleanResult = dynamic_cast<IIfcBooleanResult^>(ifcCsgSolid->TreeRootExpression);
				if (booleanResult != nullptr) return BuildBooleanResult(booleanResult);
				IIfcCsgPrimitive3D^ primitive3d = dynamic_cast<IIfcCsgPrimitive3D^>(ifcCsgSolid->TreeRootExpression);
				if (primitive3d == nullptr) throw RaiseGeometryFactoryException("Unsupported TreeRootExpression type", ifcCsgSolid);
				return BuildCsgPrimitive3D(primitive3d);
			}

			TopoDS_Solid SolidFactory::BuildBooleanResult(IIfcBooleanResult^ ifcBooleanResult)
			{
				auto shape = BOOLEAN_FACTORY->BuildBooleanResult(ifcBooleanResult);
				if(shape.IsNull())
					throw RaiseGeometryFactoryException("Failed to build Boolean Result", ifcBooleanResult);
				if(shape.ShapeType()!=TopAbs_SOLID)
					throw RaiseGeometryFactoryException("Failed to build Boolean Result as a solid", ifcBooleanResult);
				return TopoDS::Solid(shape);
			}

			TopoDS_Solid SolidFactory::BuildCsgPrimitive3D(IIfcCsgPrimitive3D^ ifcCsgPrimitive3D)
			{

				XCsgPrimitive3dType csgType;
				if (!Enum::TryParse<XCsgPrimitive3dType>(ifcCsgPrimitive3D->ExpressType->ExpressName, csgType))
					throw RaiseGeometryFactoryException("Unsupported CsgPrimitive3D type", ifcCsgPrimitive3D);
				switch (csgType)
				{
				case XCsgPrimitive3dType::IfcBlock:
					return BuildBlock(static_cast<IIfcBlock^>(ifcCsgPrimitive3D));
				case XCsgPrimitive3dType::IfcRectangularPyramid:
					return BuildRectangularPyramid(static_cast<IIfcRectangularPyramid^>(ifcCsgPrimitive3D));
				case XCsgPrimitive3dType::IfcRightCircularCone:
					return BuildRightCircularCone(static_cast<IIfcRightCircularCone^>(ifcCsgPrimitive3D));
				case XCsgPrimitive3dType::IfcRightCircularCylinder:
					return BuildRightCircularCylinder(static_cast<IIfcRightCircularCylinder^>(ifcCsgPrimitive3D));
				case XCsgPrimitive3dType::IfcSphere:
					return BuildSphere(static_cast<IIfcSphere^>(ifcCsgPrimitive3D));
				default:
					break;
				}
				throw RaiseGeometryFactoryException("Not implemented. CsgPrimitive3D type", ifcCsgPrimitive3D);

			}

			TopoDS_Solid SolidFactory::BuildBlock(IIfcBlock^ ifcBlock)
			{
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcBlock->Position, ax2)) //must be 3D according to schema
					throw RaiseGeometryFactoryException("Csg block has invalid axis placement", ifcBlock->Position);
				if (ifcBlock->XLength <= 0 || ifcBlock->YLength <= 0 || ifcBlock->ZLength <= 0)
					throw RaiseGeometryFactoryException("Csg block is a solid with zero volume", ifcBlock);
				return EXEC_NATIVE->BuildBlock(ax2, ifcBlock->XLength, ifcBlock->YLength, ifcBlock->ZLength);

			}

			TopoDS_Solid SolidFactory::BuildRectangularPyramid(IIfcRectangularPyramid^ ifcRectangularPyramid)
			{
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcRectangularPyramid->Position, ax2)) //must be 3D according to schema
					throw RaiseGeometryFactoryException("Csg rectangle pyramid has invalid axis placement", ifcRectangularPyramid->Position);
				if (ifcRectangularPyramid->XLength <= 0 || ifcRectangularPyramid->YLength <= 0 || ifcRectangularPyramid->Height <= 0)
					throw RaiseGeometryFactoryException("Csg Rectangular Pyramid is a solid with zero volume", ifcRectangularPyramid);
				return EXEC_NATIVE->BuildRectangularPyramid(ax2, ifcRectangularPyramid->XLength, ifcRectangularPyramid->YLength, ifcRectangularPyramid->Height);
			}

			TopoDS_Solid SolidFactory::BuildRightCircularCone(IIfcRightCircularCone^ ifcRightCircularCone)
			{
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcRightCircularCone->Position, ax2)) //must be 3D according to schema
					throw RaiseGeometryFactoryException("Csg circular cone has invalid axis placement", ifcRightCircularCone->Position);
				if (ifcRightCircularCone->BottomRadius <= 0 || ifcRightCircularCone->Height <= 0)
					throw RaiseGeometryFactoryException("Csg RightCircularCone is a solid with zero volume");
				return EXEC_NATIVE->BuildRightCircularCone(ax2, ifcRightCircularCone->BottomRadius, ifcRightCircularCone->Height);
			}

			TopoDS_Solid SolidFactory::BuildRightCircularCylinder(IIfcRightCircularCylinder ^ (ifcRightCircularCylinder))
			{
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcRightCircularCylinder->Position, ax2)) //must be 3D according to schema
					throw RaiseGeometryFactoryException("Csg circular cylinder has invalid axis placement", ifcRightCircularCylinder->Position);
				if (ifcRightCircularCylinder->Radius <= 0 || ifcRightCircularCylinder->Height <= 0)
					throw RaiseGeometryFactoryException("Csg RightCircularCylinder is a solid with zero volume");
				return EXEC_NATIVE->BuildRightCylinder(ax2, ifcRightCircularCylinder->Radius, ifcRightCircularCylinder->Height);
			}

			TopoDS_Solid SolidFactory::BuildSphere(IIfcSphere^ ifcSphere)
			{
				gp_Ax2 ax2;
				if (!GEOMETRY_FACTORY->BuildAxis2Placement3d(ifcSphere->Position, ax2)) //must be 3D according to schema
					throw RaiseGeometryFactoryException("Csg sphere has invalid axis placement", ifcSphere->Position);
				if (ifcSphere->Radius <= 0)
					throw RaiseGeometryFactoryException("Csg Sphere is a solid with zero volume", ifcSphere);
				return EXEC_NATIVE->BuildSphere(ax2, ifcSphere->Radius);
			}


			void SolidFactory::MoveSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Wire& section)
			{
				gp_Ax3 toAx3(loc, sectionNormal, refVec);
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				section.Move(topLoc);
			}

			void SolidFactory::MoveSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Face& section)
			{
				gp_Ax3 toAx3(loc, sectionNormal, refVec);
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				section.Move(topLoc);
			}

			TopoDS_Face SolidFactory::MovedSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Face& section)
			{
				gp_Ax3 toAx3(loc, sectionNormal, refVec);
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				BRepBuilderAPI_Transform transformer(section, topLoc, true);
				transformer.Build();
				return TopoDS::Face(transformer.Shape());
			}

			TopoDS_Wire SolidFactory::MovedSection(gp_Pnt& loc, gp_Vec& sectionNormal, gp_Vec& refVec, TopoDS_Wire& section)
			{
				gp_Ax3 toAx3(loc, sectionNormal, refVec);
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				BRepBuilderAPI_Transform transformer(section, topLoc, true);
				transformer.Build();
				auto wire = TopoDS::Wire(transformer.Shape());
				return wire;
			}
		}
	}
}