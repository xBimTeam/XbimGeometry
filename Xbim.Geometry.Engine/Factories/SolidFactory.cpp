#include "SolidFactory.h"
#include "WireFactory.h"
#include "ProfileFactory.h"
#include "ShellFactory.h"
#include "GeometryFactory.h"
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
			IXShape^ ShapeFactory::Convert(System::String^ brepStr)
			{
				const char* cStr = (const char*)(Marshal::StringToHGlobalAnsi(brepStr)).ToPointer();
				try
				{
					TopoDS_Shape shape = OccHandle().Convert(cStr);
					switch (shape.ShapeType())
					{
					case TopAbs_VERTEX:
						return gcnew XVertex(TopoDS::Vertex(shape));
					case TopAbs_EDGE:
						return gcnew XEdge(TopoDS::Edge(shape));
					case TopAbs_WIRE:
						return gcnew XWire(TopoDS::Wire(shape));
					case TopAbs_FACE:
						return gcnew XFace(TopoDS::Face(shape));
					case TopAbs_SHELL:
						return gcnew XShell(TopoDS::Shell(shape));
					case TopAbs_SOLID:
						return gcnew XSolid(TopoDS::Solid(shape));
					case TopAbs_COMPOUND:
						return  gcnew XCompound(TopoDS::Compound(shape));
					case TopAbs_COMPSOLID:
					default:
						LogError("Unsupported Shape Type, Compound Solid");
					}
				}
				catch (...)
				{
					throw gcnew XbimGeometryServiceException("Failure to convert from Brep string");
				}
				finally
				{
					Marshal::FreeHGlobal(System::IntPtr((void*)cStr));
				}
				throw gcnew XbimGeometryServiceException("Failure to convert from Brep string");
			}
			IXShape^ SolidFactory::Build(IIfcSolidModel^ ifcSolid)
			{
				TopoDS_Shape topoShape = BuildSolidModel(ifcSolid);
				if (topoShape.IsNull())
					throw RaiseGeometryFactoryException("Failure building IIfcSolidModel", ifcSolid);
				topoShape.Closed(true);
				return  ShapeFactory::GetXbimShape(topoShape);
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
				/*XCurveType directrixCurveType;
				Handle(Geom_Curve) directrix = _curveFactory->BuildDirectrix(ifcSolid->Directrix,
					ifcSolid->StartParam.HasValue ? (double)ifcSolid->StartParam.Value : -1,
					ifcSolid->EndParam.HasValue ? (double)ifcSolid->EndParam.Value : -1,
					directrixCurveType);*/
				double startParam = ifcSolid->StartParam.HasValue ? (double)ifcSolid->StartParam.Value : double::NaN;
				double endParam = ifcSolid->EndParam.HasValue ? (double)ifcSolid->EndParam.Value : double::NaN;
				TopoDS_Wire directrix = WIRE_FACTORY->BuildDirectrixWire(ifcSolid->Directrix, startParam, endParam);
				if (directrix.IsNull())
					throw RaiseGeometryFactoryException("Could not build directrix", ifcSolid);
				double innerRadius = ifcSolid->InnerRadius.HasValue ? (double)ifcSolid->InnerRadius.Value : -1;
				BRepBuilderAPI_TransitionMode transitionMode = BRepBuilderAPI_TransitionMode::BRepBuilderAPI_Transformed;
				//With Polyline the consecutive segments of the Directrix are not tangent continuous, the resulting solid is created by a miter at half angle between the two segments.
				if (dynamic_cast<IIfcPolyline^>(ifcSolid->Directrix))
					transitionMode = BRepBuilderAPI_TransitionMode::BRepBuilderAPI_RightCorner;

				TopoDS_Solid solid = Ptr()->BuildSweptDiskSolid(directrix, ifcSolid->Radius, innerRadius, transitionMode);
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
				TopoDS_Solid solid = EXEC_NATIVE->BuildExtrudedAreaSolid(sweptArea, extrudeDirection, extrudedSolid->Depth);
				if (solid.IsNull() || solid.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Extruded Solid could not be built", extrudedSolid);
				return solid;
			}

			TopoDS_Solid SolidFactory::BuildFacetedBrep(IIfcFacetedBrep^ facetedBrep)
			{
				CheckClosedStatus isCheckedClosed;
				TopoDS_Shell shell =  SHELL_FACTORY->BuildClosedShell(facetedBrep->Outer, isCheckedClosed); //throws exeptions
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
				TopoDS_Shell shell =   SHELL_FACTORY->BuildClosedShell(ifcAdvancedBrep->Outer, isCheckedClosed);
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