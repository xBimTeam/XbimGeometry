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
					Handle(Geom_Plane) plane = SURFACE_FACTORY->BuildPlane(planarSurface);
					gp_Vec normalDir = plane->Axis().Direction();
					if (ifcHalfSpaceSolid->AgreementFlag) normalDir.Reverse();
					pointInMaterial = plane->Location().Translated(normalDir * _modelService->OneMeter);
					face = BRepBuilderAPI_MakeFace(plane, _modelService->Precision);
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
					if(wire.IsNull())
						throw RaiseGeometryFactoryException("IfcPolygonalBoundedHalfSpace polygonal boundary could not be built", ifcHalfSpaceSolid);
					auto face = FACE_FACTORY->EXEC_NATIVE->BuildProfileDef(gp_Pln(), wire);
					TopLoc_Location location;
					if (polygonalBoundedHalfSpace->Position != nullptr)
						location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(polygonalBoundedHalfSpace->Position);
					TopoDS_Solid substractionBody = EXEC_NATIVE->MakeSweptSolid(face, gp_Vec(0, 0, 1e1));
					gp_Trsf shift;
					shift.SetTranslation(gp_Vec(0, 0, -1e1 / 2));
					substractionBody.Move(shift);
					substractionBody.Move(location);
					bool hasWarnings;
					TopoDS_Shape shape = BOOLEAN_FACTORY->EXEC_NATIVE->Intersect( halfSpace,substractionBody, ModelGeometryService->MinimumGap,hasWarnings);
					if (hasWarnings)
						LogWarning(polygonalBoundedHalfSpace, "Warnings generated building half space solid. See logs");
					halfSpace = EXEC_NATIVE->CastToSolid(shape);
					if (halfSpace.IsNull())
						throw RaiseGeometryFactoryException("Failure building IfcPolygonBoundedHalfSpaceSolid", polygonalBoundedHalfSpace);
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

			IXSolid^ SolidFactory::Build(IIfcExtrudedAreaSolid^ ifcExtrudedAreaSolid)
			{
				TopoDS_Solid solid = BuildExtrudedAreaSolid(ifcExtrudedAreaSolid);
				return  gcnew XSolid(solid);
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
					//srl the following methods will need to be implemented as Version 6, defaulting to version 5 implementation
				case XSolidModelType::IfcSweptDiskSolidPolygonal:
					return gcnew XbimSolid(static_cast<IIfcSweptDiskSolidPolygonal^>(ifcSolid), Logger());
				case XSolidModelType::IfcAdvancedBrep:
					return gcnew XbimSolid(static_cast<IIfcAdvancedBrep^>(ifcSolid), Logger());
				case XSolidModelType::IfcAdvancedBrepWithVoids:
					return gcnew XbimSolid(static_cast<IIfcAdvancedBrepWithVoids^>(ifcSolid), Logger());
				case XSolidModelType::IfcFacetedBrepWithVoids:
					return gcnew XbimSolid(static_cast<IIfcFacetedBrepWithVoids^>(ifcSolid), Logger());
				case XSolidModelType::IfcExtrudedAreaSolidTapered:
					return gcnew XbimSolid(static_cast<IIfcExtrudedAreaSolidTapered^>(ifcSolid), Logger());
				case XSolidModelType::IfcFixedReferenceSweptAreaSolid:
					return gcnew XbimSolid(static_cast<IIfcFixedReferenceSweptAreaSolid^>(ifcSolid), Logger());
				case XSolidModelType::IfcRevolvedAreaSolid:
					return gcnew XbimSolid(static_cast<IIfcRevolvedAreaSolid^>(ifcSolid), Logger());
				case XSolidModelType::IfcRevolvedAreaSolidTapered:
					return gcnew XbimSolid(static_cast<IIfcRevolvedAreaSolidTapered^>(ifcSolid), Logger());
				case XSolidModelType::IfcSurfaceCurveSweptAreaSolid:
					return gcnew XbimSolid(static_cast<IIfcSurfaceCurveSweptAreaSolid^>(ifcSolid), Logger());
				default:
					break;
				}
				throw RaiseGeometryFactoryException("Not implemented. SolidModel type", ifcSolid);
			}

			TopoDS_Solid SolidFactory::BuildSweptDiskSolid(IIfcSweptDiskSolid^ ifcSolid)
			{
				if (ifcSolid->Radius <= 0)
					throw RaiseGeometryFactoryException("Radius must be greater than 0", ifcSolid);
				if (ifcSolid->InnerRadius.HasValue && ifcSolid->InnerRadius.Value >= ifcSolid->Radius)
					throw RaiseGeometryFactoryException("Inner radius is greater than outer radius", ifcSolid);

				Handle(Geom_Curve) directrix = CURVE_FACTORY->BuildDirectrixCurve(ifcSolid->Directrix, ifcSolid->StartParam, ifcSolid->EndParam);
				if (directrix.IsNull())
					throw RaiseGeometryFactoryException("Could not build directrix", ifcSolid);
				double innerRadius = ifcSolid->InnerRadius.HasValue ? (double)ifcSolid->InnerRadius.Value : -1;


				TopoDS_Solid solid = EXEC_NATIVE->BuildSweptDiskSolid(directrix, ifcSolid->Radius, innerRadius);
				return solid;

			}

			TopoDS_Solid SolidFactory::BuildExtrudedAreaSolid(IIfcExtrudedAreaSolid^ extrudedSolid)
			{
				if (extrudedSolid->Depth <= 0)
					throw RaiseGeometryFactoryException("Extruded Solid depth must be greater than 0", extrudedSolid);

				gp_Vec extrudeDirection;
				if (!GEOMETRY_FACTORY->BuildDirection3d(extrudedSolid->ExtrudedDirection, extrudeDirection))
					throw RaiseGeometryFactoryException("Extruded Solid sweep dirction is illegal", extrudedSolid->ExtrudedDirection);
				TopoDS_Face sweptArea = PROFILE_FACTORY->BuildProfileFace(extrudedSolid->SweptArea); //if this fails it will throw an exception
				if (sweptArea.IsNull())
					throw RaiseGeometryFactoryException("Extruded Solid Swept area could not be built", extrudedSolid->SweptArea);
				TopLoc_Location location;
				if (extrudedSolid->Position != nullptr)
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(extrudedSolid->Position);
				TopoDS_Solid solid = EXEC_NATIVE->BuildExtrudedAreaSolid(sweptArea, extrudeDirection, extrudedSolid->Depth, location);
				if (solid.IsNull() || solid.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Extruded Solid could not be built", extrudedSolid);
				
				return solid;
			}

			TopoDS_Solid SolidFactory::BuildFacetedBrep(IIfcFacetedBrep^ facetedBrep)
			{
				CheckClosedStatus isCheckedClosed;
				TopoDS_Shell shell = SHELL_FACTORY->BuildClosedShell(facetedBrep->Outer, isCheckedClosed); //throws exeptions
				return EXEC_NATIVE->MakeSolid(shell);
			}


			/// <summary>
			/// FC4 CHANGE  The entity has been deprecated and shall not be used. The entity IfcFacetedBrep shall be used instead. Implemented for backward compatibility
			/// </summary>
			/// <param name="faceBasedSurfaceModel"></param>
			/// <returns></returns>
			TopoDS_Compound SolidFactory::BuildFaceBasedSurfaceModel(IIfcFaceBasedSurfaceModel^ faceBasedSurfaceModel)
			{
				TopoDS_Compound compound;
				BRep_Builder builder;
				builder.MakeCompound(compound);
				ShapeFix_Shell myFixShell;
				myFixShell.FixFaceMode() = false; //we don't want to change any faces
				//myFixShell.FixOrientationMode() = false;
				for each (IIfcConnectedFaceSet ^ faceSet in faceBasedSurfaceModel->FbsmFaces)
				{
					CheckClosedStatus isCheckedClosed;
					TopoDS_Shell shell = SHELL_FACTORY->BuildConnectedFaceSet(faceSet, isCheckedClosed);
					switch (isCheckedClosed)
					{
					case CheckAndClosed:
						builder.Add(compound, EXEC_NATIVE->MakeSolid(shell));
						break;
					case CheckedNotClosed:		//Nb 	IfcFaceBasedSurfaceModel do not require to be made of solids or add up to a solid		
					case NotChecked:
					default:
						builder.Add(compound, shell);
						break;
					}
					//BRepCheck_Shell checker(shell);
					//BRepCheck_Status st = checker.Closed();
					//if (st == BRepCheck_Status::BRepCheck_NoError)
					//{
					//	//make it a solid
					//	TopoDS_Solid solid;
					//	builder.MakeSolid(solid);
					//	builder.Add(solid, shell);
					//	solid.Checked(true);
					//	solid.Closed(true);
					//	builder.Add(compound, solid);
					//}
					//else //try and fix the shells, often there are multiple shells in one that are solid parts
					//{
					//	myFixShell.Init(TopoDS::Shell(shell));
					//	bool ok = myFixShell.Perform();

					//	if (ok && myFixShell.NbShells() > 0)
					//	{
					//		for (TopExp_Explorer aExpSh(myFixShell.Shape(), TopAbs_SHELL); aExpSh.More(); aExpSh.Next())
					//		{
					//			TopoDS_Shell sh = TopoDS::Shell(aExpSh.Current());
					//			BRepCheck_Shell checker2(sh);
					//			BRepCheck_Status st2 = checker2.Closed();
					//			if (st2 == BRepCheck_Status::BRepCheck_NoError)
					//			{
					//				//make it a solid
					//				TopoDS_Solid solid;
					//				builder.MakeSolid(solid);
					//				builder.Add(solid, sh);
					//				solid.Checked(true);
					//				solid.Closed(true);
					//				builder.Add(compound, solid);
					//			}
					//			else
					//				builder.Add(compound, sh); //add the shell
					//		}
					//	}
					//	else
					//		builder.Add(compound, shell); //add the shell
					//}

				}
				return compound;
			}
			TopoDS_Solid SolidFactory::BuildPolygonalFaceSet(IIfcPolygonalFaceSet^ ifcPolygonalFaceSet)
			{
				CheckClosedStatus isCheckedClosed;
				TopoDS_Shell shell = SHELL_FACTORY->BuildPolygonalFaceSet(ifcPolygonalFaceSet, isCheckedClosed);
				return EXEC_NATIVE->MakeSolid(shell);
			}
			;

			TopoDS_Solid SolidFactory::BuildAdvancedBrep(IIfcAdvancedBrep^ ifcAdvancedBrep)
			{
				CheckClosedStatus isCheckedClosed;
				TopoDS_Shell shell = SHELL_FACTORY->BuildClosedShell(ifcAdvancedBrep->Outer, isCheckedClosed);
				return EXEC_NATIVE->MakeSolid(shell);
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
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
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

		}
	}
}