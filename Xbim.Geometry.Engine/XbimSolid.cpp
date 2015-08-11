#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"
#include "XbimWire.h"
#include "XbimCompound.h"
#include "XbimSolidSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimShellSet.h"
#include "XbimFaceSet.h"
#include "XbimPoint3DWithTolerance.h"
#include "XbimFacetedSolid.h"
#include "XbimGeometryCreator.h"
#include "XbimGeomPrim.h"
#include "XbimOccWriter.h"

#include <TopExp.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GC_MakeEllipse.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeLine.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgo_Loop.hxx>

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepAlgo_NormalProjection.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom_Curve.hxx>
#include <BRep_Tool.hxx>
#include <BRepPrim_Builder.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <ShapeFix_Face.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TShort_Array1OfShortReal.hxx>

#include <BRepBuilderAPI_Transform.hxx>
#include <ShapeFix_Solid.hxx>
#include <ShapeFix_Wireframe.hxx>

using namespace System;
using namespace System::Threading;
using namespace Xbim::Common;

namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimSolid::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Solid*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

#pragma region Constructors

		XbimSolid::XbimSolid(const TopoDS_Solid& solid)
		{
			pSolid = new TopoDS_Solid();
			*pSolid = solid;
		}

		XbimSolid::XbimSolid(IfcSolidModel^ solid)
		{
			Init(solid);
		}

		XbimSolid::XbimSolid(IfcSweptAreaSolid^ repItem)
		{
			Init(repItem);
		}
		XbimSolid::XbimSolid(IfcRevolvedAreaSolid^ solid)
		{
			Init(solid);
		}

		XbimSolid::XbimSolid(IfcExtrudedAreaSolid^ repItem)
		{
			Init(repItem);
			
		}
		
		XbimSolid::XbimSolid(IfcSweptDiskSolid^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcBoundingBox^ repItem)
		{
			Init(repItem);
		}
		
		XbimSolid::XbimSolid(IfcSurfaceCurveSweptAreaSolid^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcHalfSpaceSolid^ repItem, double maxExtrusion)
		{
			Init(repItem, maxExtrusion);
		}

		XbimSolid::XbimSolid(IfcBoxedHalfSpace^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(XbimRect3D rect3D, double tolerance)
		{
			Init(rect3D, tolerance);
		}

		XbimSolid::XbimSolid(IfcPolygonalBoundedHalfSpace^ repItem, double maxExtrusion)
		{
			Init(repItem, maxExtrusion);
		}

		XbimSolid::XbimSolid(IfcBooleanClippingResult^ solid)
		{
			Init(solid);
		}

		XbimSolid::XbimSolid(IfcBooleanOperand^ repItem)
		{
			Init(repItem);
		}

		

		XbimSolid::XbimSolid(IfcCsgPrimitive3D^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcCsgSolid^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcSphere^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcBlock^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcRightCircularCylinder^ repItem)
		{
			Init(repItem);
		}

		XbimSolid::XbimSolid(IfcRightCircularCone^ repItem)
		{
			Init(repItem);
		}
		
		XbimSolid::XbimSolid(IfcRectangularPyramid^ repItem)
		{
			Init(repItem);
		}
#pragma   endregion


#pragma region Equality Overrides

		bool XbimSolid::Equals(Object^ obj)
		{
			// Check for null
			if (obj == nullptr) return false;

			// Check for type
			if (this->GetType() != obj->GetType()) return false;

			// Cast as XbimVertex
			XbimSolid^ s = (XbimSolid^)obj;
			return this == s;
		}

		bool XbimSolid::Equals(IXbimSolid^ obj)
		{
			// Check for null
			if (obj == nullptr) return false;
			return this == (XbimSolid^)obj;
		}

		int XbimSolid::GetHashCode()
		{
			if (!IsValid) return 0;
			return pSolid->HashCode(Int32::MaxValue);
		}

		bool XbimSolid::operator ==(XbimSolid^ left, XbimSolid^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  ((const TopoDS_Solid&)left).IsEqual(right) == Standard_True;

		}

		bool XbimSolid::operator !=(XbimSolid^ left, XbimSolid^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region Initialisation methods

		void XbimSolid::Init(IfcSolidModel^ solid)
		{
			IfcSweptAreaSolid^ extrudeArea = dynamic_cast<IfcSweptAreaSolid^>(solid);
			if (extrudeArea) return Init(extrudeArea);
			IfcSweptDiskSolid^ sd = dynamic_cast<IfcSweptDiskSolid^>(solid);
			if (sd != nullptr) return Init(sd);
			IfcManifoldSolidBrep^ ms = dynamic_cast<IfcManifoldSolidBrep^>(solid);
			if (ms != nullptr) return Init(ms);
			IfcCsgSolid^ csg = dynamic_cast<IfcCsgSolid^>(solid);
			if (csg != nullptr) return Init(csg);
			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", solid->GetType()->Name, solid->EntityLabel));
		}

		void XbimSolid::Init(IfcSweptAreaSolid^ solid)
		{
			IfcExtrudedAreaSolid^ extrudeArea = dynamic_cast<IfcExtrudedAreaSolid^>(solid);
			if (extrudeArea) return Init(extrudeArea);
			IfcRevolvedAreaSolid^ ras = dynamic_cast<IfcRevolvedAreaSolid^>(solid);
			if (ras != nullptr) return Init(ras);
			
			XbimGeometryCreator::logger->WarnFormat("WS017: Swept Solid of Type {0} in entity #{1} is not implemented", solid->GetType()->Name, solid->EntityLabel);
		}

		void XbimSolid::Init(IfcSurfaceCurveSweptAreaSolid^ repItem)
		{
			XbimFace^ profile = gcnew XbimFace(repItem->SweptArea);
			if (!profile->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS018: Could not build Swept Area of IfcSurfaceCurveSweptAreaSolid #{0}", repItem->EntityLabel);
				return;
			}
			
			XbimModelFactors^ mf = repItem->ModelOf->ModelFactors;
			XbimWire^ sweep = gcnew XbimWire(repItem->Directrix);
			sweep = (XbimWire^)sweep->Trim(repItem->StartParam, repItem->EndParam, mf->Precision);
			if (!sweep->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS019: Could not build Directrix of IfcSurfaceCurveSweptAreaSolid #{0}", repItem->EntityLabel);
				return;
			}
			BRepOffsetAPI_MakePipeShell pipeMaker1(sweep); 

			if (dynamic_cast<IfcPlane^>(repItem->ReferenceSurface))
			{
				IfcPlane^ ifcPlane = (IfcPlane^)repItem->ReferenceSurface;
				gp_Ax3 ax3 = XbimGeomPrim::ToAx3(ifcPlane->Position);
				pipeMaker1.SetMode(ax3.Direction()); 
				//find the start position of the sweep
				
				BRepTools_WireExplorer wExp(sweep);
				Standard_Real start = 0;
				Standard_Real end = 1;
				Handle_Geom_Curve curve = BRep_Tool::Curve(wExp.Current(), start, end);
				gp_Pnt p1;
				gp_Vec tangent;
				curve->D1(0, p1, tangent);
				const TopoDS_Vertex firstPoint = wExp.CurrentVertex();
				gp_Ax3 toAx3(BRep_Tool::Pnt(firstPoint), tangent, ax3.Direction());	//rotate so normal of profile is tangental and X axis 
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				XbimWire^ outerBound = (XbimWire^)(profile->OuterBound);
				TopoDS_Wire p = outerBound;
				p.Location(topLoc);
				
				pipeMaker1.SetTransitionMode(BRepBuilderAPI_RightCorner); 
				pipeMaker1.Add(p, Standard_False, Standard_False);
				pipeMaker1.Build();
				if (pipeMaker1.IsDone() && pipeMaker1.MakeSolid())
				{		
					//do any inner loops					
					BRepPrim_Builder b;
					TopoDS_Shell shell;
					b.MakeShell(shell);
					XbimFace^ bottomOuter = gcnew XbimFace(TopoDS::Face(pipeMaker1.FirstShape()));
					XbimFace^ topOuter = gcnew XbimFace(TopoDS::Face(pipeMaker1.LastShape()));
					const TopoDS_Shape& pipeOuter = pipeMaker1.Shape();
					for (TopExp_Explorer explr(pipeOuter, TopAbs_FACE); explr.More(); explr.Next())
					{
						const TopoDS_Face& face = TopoDS::Face(explr.Current());
						//Opencascade 6.8 changed first and lastshape so that isEqual failed, there is an issue with orientation so use isSame 
						if (!face.IsSame(pipeMaker1.FirstShape()) && !face.IsSame(pipeMaker1.LastShape()))
							b.AddShellFace(shell, face);
					}

					for each (IXbimWire^ innerBound in profile->InnerBounds) //only do the first
					{
						BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
						pipeMaker2.SetMode(ax3.Direction());
						pipeMaker2.SetTransitionMode(BRepBuilderAPI_RightCorner);

						XbimWire^ xInnerBound = (XbimWire^)innerBound;
						TopoDS_Wire i = xInnerBound;
						i.Location(topLoc);
						pipeMaker2.Add(i, Standard_False, Standard_False);
						pipeMaker2.Build();
						if (pipeMaker2.IsDone() && pipeMaker2.MakeSolid())
						{
							XbimFace^ bottomInner = gcnew XbimFace(TopoDS::Face(pipeMaker2.FirstShape()));
							XbimFace^ topInner = gcnew XbimFace(TopoDS::Face(pipeMaker2.LastShape()));
							//add the inner loops to the end faces
							bottomOuter->Add(bottomInner->OuterBound);
							topOuter->Add(topInner->OuterBound);
							//add the other faces to the shell
							for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
							{
								const TopoDS_Face& face = TopoDS::Face(explr.Current());
								//Opencascade 6.8 changed first and lastshape so that isEqual failed, there is an issue with orientation so use isSame 
								if (!face.IsSame(pipeMaker2.FirstShape()) && !face.IsSame(pipeMaker2.LastShape()))
									b.AddShellFace(shell, face);
							}
						}
						break; //only do one
					}
					//add top and bottom faces with their hole loops
					b.AddShellFace(shell, bottomOuter);
					b.AddShellFace(shell, topOuter);
					b.CompleteShell(shell);
					BRep_Builder bs;
					pSolid = new TopoDS_Solid();
					bs.MakeSolid(*pSolid);
					bs.Add(*pSolid, shell);
					Move(repItem->Position);
					return;

				}
				else
				{
					XbimGeometryCreator::logger->WarnFormat("Entity #" + repItem->EntityLabel.ToString() + ", IfcSurfaceCurveSweptAreaSolid could not be constructed ");
				}

			}
			else
			{
				XbimGeometryCreator::logger->WarnFormat("WS019: Entity #" + repItem->EntityLabel.ToString() + ", IfcSurfaceCurveSweptAreaSolid has a Non-Planar surface");
				pipeMaker1.SetMode(Standard_False); //use auto calculation of tangent and binormal
				pipeMaker1.Add(profile, Standard_False, Standard_True);
				pipeMaker1.Build();
				if (pipeMaker1.IsDone() && pipeMaker1.MakeSolid())
				{
					TopoDS_Shape result = pipeMaker1.Shape();
					result.Move(XbimGeomPrim::ToLocation(repItem->Position));
					pSolid = new TopoDS_Solid();
					*pSolid = TopoDS::Solid(result);
					return;
				}
			}	
		}

		void XbimSolid::Init(IfcExtrudedAreaSolid^ repItem)
		{
			XbimFace^ face = gcnew XbimFace(repItem->SweptArea);
			if (face->IsValid && repItem->Depth > 0) //we have a valid face and extrusion
			{
				IfcDirection^ dir = repItem->ExtrudedDirection;
				gp_Vec vec(dir->X, dir->Y, dir->Z);
				vec *= repItem->Depth;
				BRepPrimAPI_MakePrism prism(face, vec);
				GC::KeepAlive(face);
				if (prism.IsDone())
				{
					pSolid = new TopoDS_Solid();
					*pSolid = TopoDS::Solid(prism.Shape());
					pSolid->Move(XbimGeomPrim::ToLocation(repItem->Position));
					//BRepTools::Write(*pSolid, "d:\\tmp\\b");
				}
				else
					XbimGeometryCreator::logger->WarnFormat("WS002: Invalid Solid Extrusion, could not create solid, found in Entity #{0}=IfcExtrudedAreaSolid.",
					repItem->EntityLabel);
			}
			else if (repItem->Depth <= 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WS001: Invalid Solid Extrusion, Extrusion Depth must be >0, found in Entity #{0}=IfcExtrudedAreaSolid.",
					repItem->EntityLabel);
			}
			//if it has failed we will have a null solid
		}

		void XbimSolid::Init(IfcRevolvedAreaSolid^ repItem)
		{
			XbimFace^ face = gcnew XbimFace(repItem->SweptArea);

			if (face->IsValid && repItem->Angle > 0) //we have a valid face and angle
			{
				IfcAxis1Placement^ revolaxis = repItem->Axis;
				gp_Pnt origin(revolaxis->Location->X, revolaxis->Location->Y, revolaxis->Location->Z);
				gp_Dir vx(revolaxis->Axis->X, revolaxis->Axis->Y, revolaxis->Axis->Z);
				gp_Ax1 ax1(origin, vx);
				double radianConvert = repItem->ModelOf->ModelFactors->AngleToRadiansConversionFactor;
				BRepPrimAPI_MakeRevol revol(face, ax1, repItem->Angle*radianConvert);
				
				GC::KeepAlive(face);
				if (revol.IsDone())
				{
					//BRepTools::Write(revol.Shape(), "d:\\tmp\\rev");
					pSolid = new TopoDS_Solid();
					*pSolid = TopoDS::Solid(revol.Shape());
					pSolid->Move(XbimGeomPrim::ToLocation(repItem->Position));
				}
				else
					XbimGeometryCreator::logger->WarnFormat("WS003: Invalid Solid Extrusion, could not create solid, found in Entity #{0}=IfcRevolvedAreaSolid.",
					repItem->EntityLabel);
			}
			else if (repItem->Angle <= 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WS004: Invalid Solid Extrusion, Extrusion Angle must be >0, found in Entity #{0}=IfcRevolvedAreaSolid.",
					repItem->EntityLabel);
			}
		}

		void XbimSolid::Init(IfcHalfSpaceSolid^ hs, double maxExtrusion)
		{
			if (dynamic_cast<IfcPolygonalBoundedHalfSpace^>(hs))
				return Init((IfcPolygonalBoundedHalfSpace^)hs, maxExtrusion);
			else if (dynamic_cast<IfcBoxedHalfSpace^>(hs))
				return Init((IfcBoxedHalfSpace^)hs);
			else //it is a simple Half space
			{
				IfcSurface^ surface = (IfcSurface^)hs->BaseSurface;
				IfcPlane^ ifcPlane = dynamic_cast<IfcPlane^>(surface);
				if (ifcPlane == nullptr)
				{
					XbimGeometryCreator::logger->WarnFormat("WS011: Non-Planar half spaces are not supported in Entity #{0}, it has been ignored", hs->EntityLabel);
					return;
				}
#ifdef OCC_6_9_SUPPORTED


				gp_Ax3 ax3 = XbimGeomPrim::ToAx3(ifcPlane->Position);
				gp_Pln pln(ax3);
				gp_Vec zVec = hs->AgreementFlag ? -pln.Axis().Direction() : pln.Axis().Direction();

				gp_Pnt pnt = ax3.Location();
				pnt.Translate(zVec);
				BRepBuilderAPI_MakeFace  faceMaker(pln);
				if (faceMaker.IsDone())
				{
					BRepPrimAPI_MakeHalfSpace halfSpaceBulder(faceMaker.Face(), pnt);
					pSolid = new TopoDS_Solid();
					*pSolid = TopoDS::Solid(halfSpaceBulder.Solid());
				}

#else


				double z = hs->AgreementFlag ? -2e8 : 0;
				XbimPoint3D corner(-1e8, -1e8, z);
				 
				XbimVector3D size(2e8, 2e8, 2e8);
				XbimRect3D rect3D(corner, size);
				Init(rect3D, hs->ModelOf->ModelFactors->Precision);
				Move(ifcPlane->Position);
#endif
			}
		}

		void XbimSolid::Init(XbimRect3D rect3D, double tolerance)
		{
			
			XbimPoint3D l = rect3D.Location;		
			BRepPrimAPI_MakeBox box(gp_Pnt(l.X, l.Y, l.Z), rect3D.SizeX, rect3D.SizeY, rect3D.SizeZ);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(box.Shape());
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(*pSolid,tolerance, TopAbs_VERTEX);
		}

		void XbimSolid::Init(IfcBoxedHalfSpace^ bhs)
		{
			IfcSurface^ surface = (IfcSurface^)bhs->BaseSurface;
			if (!dynamic_cast<IfcPlane^>(surface))
			{
				XbimGeometryCreator::logger->WarnFormat("WS011: Non-Planar half spaces are not supported in Entity #{0}, it has been ignored", bhs->EntityLabel);
				return;
			}
			IfcPlane^ ifcPlane = (IfcPlane^)surface;
			
			Init(bhs->Enclosure);
			if (bhs->AgreementFlag)
				Translate(XbimVector3D(0, 0, -bhs->Enclosure->ZDim));
			Move(ifcPlane->Position);
		}

		void XbimSolid::Init(IfcPolygonalBoundedHalfSpace^ pbhs, double extrusionMax)
		{

			IfcSurface^ surface = (IfcSurface^)pbhs->BaseSurface;
			if (!dynamic_cast<IfcPlane^>(surface))
			{
				XbimGeometryCreator::logger->WarnFormat("WS011: Non-Planar half spaces are not supported in Entity #{0}, it has been ignored", pbhs->EntityLabel);
				return;
			}
			IfcPlane^ ifcPlane = (IfcPlane^)surface;
			gp_Ax3 ax3 = XbimGeomPrim::ToAx3(ifcPlane->Position);
			gp_Pln pln(ax3);			
			const gp_Pnt pnt = pln.Location().Translated(pbhs->AgreementFlag ? -pln.Axis().Direction() : pln.Axis().Direction());
			TopoDS_Shape halfspace = BRepPrimAPI_MakeHalfSpace(BRepBuilderAPI_MakeFace(pln), pnt).Solid();

			XbimWire^ polyBoundary = gcnew XbimWire(pbhs->PolygonalBoundary);
			
			if (!polyBoundary->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS005: The IfcPolygonalBoundedHalfSpace #{0} has an incorrectly defined PolygonalBoundary #{1}, it has been ignored", pbhs->EntityLabel, pbhs->PolygonalBoundary->EntityLabel);
				return;
			}
			//BRepTools::Write(polyBoundary, "d:\\tmp\\w1");
			//removes any colinear edges that might generate unnecessary detail and confusion for boolean operations
			if (polyBoundary->Edges->Count>4) //may sure we remove an colinear edges
				polyBoundary->FuseColinearSegments(pbhs->ModelOf->ModelFactors->Precision, 0.05);
			//BRepTools::Write(polyBoundary, "d:\\tmp\\w2");
			XbimFace^ polyFace = gcnew XbimFace(polyBoundary);

			if (!polyFace->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS009: The IfcPolygonalBoundedHalfSpace #{0} has an incorrectly defined Face with PolygonalBoundary #{1}, it has been ignored", pbhs->EntityLabel, pbhs->PolygonalBoundary->EntityLabel);
				return;
			}
			TopoDS_Shape boundedHalfSpace = BRepPrimAPI_MakePrism(polyFace, gp_Vec(0, 0, extrusionMax*4));
			gp_Trsf trsf = XbimGeomPrim::ToTransform(pbhs->Position);
			gp_Trsf offset; 
			offset.SetTranslation(gp_Vec(0, 0, -(extrusionMax/2 )));
			boundedHalfSpace.Move(trsf*offset);
			//BRepTools::Write(boundedHalfSpace, "d:\\tmp\\bh");
			//BRepTools::Write(halfspace, "d:\\tmp\\hs");
			TopoDS_Shape result = BRepAlgoAPI_Common(boundedHalfSpace,halfspace);
			
			for (TopExp_Explorer explr(result, TopAbs_SOLID); explr.More(); explr.Next())
			{
				pSolid = new TopoDS_Solid();
				*pSolid = TopoDS::Solid(explr.Current()); //just take the first solid
				//BRepTools::Write(*pSolid, "d:\\tmp\\r");
				return;
			}
			GC::KeepAlive(polyFace);
			XbimGeometryCreator::logger->WarnFormat("WS010: Failed to create IfcPolygonalBoundedHalfSpace #{0}", pbhs->EntityLabel);

		}


		void XbimSolid::Init(IfcSweptDiskSolid^ swdSolid)
		{

			//Build the directrix
			XbimModelFactors^ mf = swdSolid->ModelOf->ModelFactors;
			XbimWire^ sweep = gcnew XbimWire(swdSolid->Directrix);
			sweep = (XbimWire^)sweep->Trim(swdSolid->StartParam, swdSolid->EndParam, mf->Precision);
			
			//make the outer wire
			XbimPoint3D s = sweep->Start;
			gp_Ax2 axCircle(gp_Pnt(s.X, s.Y, s.Z), gp_Dir(0., 0., 1.));
			gp_Circ outer(axCircle, swdSolid->Radius);
			Handle(Geom_Circle) hOuter = GC_MakeCircle(outer);
			TopoDS_Edge outerEdge = BRepBuilderAPI_MakeEdge(hOuter);
			BRepBuilderAPI_MakeWire outerWire;
			outerWire.Add(outerEdge);
			BRepOffsetAPI_MakePipeShell pipeMaker(sweep);
			pipeMaker.Add(outerWire.Wire(), Standard_False, Standard_True);
			pipeMaker.Build();
			if (pipeMaker.IsDone() && pipeMaker.MakeSolid())
			{	
				BRepPrim_Builder b;
				TopoDS_Shell shell;
				b.MakeShell(shell);
				XbimFace^ bottomOuter = gcnew XbimFace(TopoDS::Face(pipeMaker.FirstShape()));
				XbimFace^ topOuter = gcnew XbimFace(TopoDS::Face(pipeMaker.LastShape()));
				const TopoDS_Shape& pipeOuter = pipeMaker.Shape();
				for (TopExp_Explorer explr(pipeOuter, TopAbs_FACE); explr.More(); explr.Next())
				{
					const TopoDS_Face& face = TopoDS::Face(explr.Current());
					if (!face.IsSame(pipeMaker.FirstShape()) && !face.IsSame(pipeMaker.LastShape()))
						b.AddShellFace(shell, face);
				}
				//now add inner wire if it is defined
				if (swdSolid->InnerRadius.HasValue && swdSolid->InnerRadius.Value > 0)
				{
					gp_Circ inner(axCircle, swdSolid->InnerRadius.Value);
					Handle(Geom_Circle) hInner = GC_MakeCircle(inner);
					TopoDS_Edge innerEdge = BRepBuilderAPI_MakeEdge(hInner);
					BRepBuilderAPI_MakeWire innerWire;
					innerWire.Add(innerEdge);
					BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
					pipeMaker2.Add(innerWire.Wire(), Standard_False, Standard_True);
					pipeMaker2.Build();
					if (pipeMaker2.IsDone() && pipeMaker2.MakeSolid())
					{

						XbimFace^ bottomInner = gcnew XbimFace(TopoDS::Face(pipeMaker2.FirstShape()));
						XbimFace^ topInner = gcnew XbimFace(TopoDS::Face(pipeMaker2.LastShape()));
						//add the inner loops to the end faces
						bottomOuter->Add(bottomInner->OuterBound);
						topOuter->Add(topInner->OuterBound);
						//add the other faces to the shell
						for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
						{
							const TopoDS_Face& face = TopoDS::Face(explr.Current());
							if (!face.IsSame(pipeMaker2.FirstShape()) && !face.IsSame(pipeMaker2.LastShape()))
								b.AddShellFace(shell, face);
						}
					}
					else
					{
						XbimGeometryCreator::logger->WarnFormat("WS022: IfcSweptDiskSolid #{0} inner loop could not be constructed", swdSolid->EntityLabel);
					}
				}
				//add top and bottom faces with their hole loops
				b.AddShellFace(shell, bottomOuter);
				b.AddShellFace(shell, topOuter);
				b.CompleteShell(shell);
				BRep_Builder bs;
				pSolid = new TopoDS_Solid();
				bs.MakeSolid(*pSolid);
				bs.Add(*pSolid, shell);
				return;
			}
			else
			{
				XbimGeometryCreator::logger->WarnFormat("WS021: IfcSweptDiskSolid #{0} could not be constructed", swdSolid->EntityLabel);
				
			}
		}

		void XbimSolid::Init(IfcBoundingBox^ box)
		{
			gp_Ax2 	gpax2(gp_Pnt(box->Corner->X, box->Corner->Y, box->Corner->Z), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
			BRepPrimAPI_MakeBox boxMaker(gpax2, box->XDim, box->YDim, box->ZDim);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(boxMaker.Shape());
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(*pSolid, box->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
		}


		XbimSolid^ XbimSolid::BuildClippingList(IfcBooleanClippingResult^ solid, List<IfcBooleanOperand^>^ clipList)
		{
			IfcBooleanOperand^ fOp = solid->FirstOperand;
			IfcBooleanOperand^ sOp = solid->SecondOperand;
			
			IfcBooleanClippingResult^ boolClip = dynamic_cast<IfcBooleanClippingResult^>(fOp);
			if (boolClip!=nullptr)
			{				
				clipList->Add(sOp);
				return XbimSolid::BuildClippingList(boolClip, clipList);
			}
			else //we need to build the solid
			{				
				clipList->Add(sOp);
				XbimSolidSet^ solidSet = dynamic_cast<XbimSolidSet^>(clipList);
				if (solidSet!=nullptr) solidSet->Reverse();
				return gcnew XbimSolid(fOp);
			}
		}

		//Booleans
		void XbimSolid::Init(IfcBooleanClippingResult^ solid)
		{
			XbimModelFactors^ mf = solid->ModelOf->ModelFactors;
			IfcBooleanOperand^ fOp = solid->FirstOperand;
#ifdef OCC_6_9_SUPPORTED			
			IfcBooleanClippingResult^ boolClip = dynamic_cast<IfcBooleanClippingResult^>(fOp);
			if (boolClip != nullptr)
			{
				List<IfcBooleanOperand^>^ clips = gcnew List<IfcBooleanOperand^>();

				IXbimSolidSet^ solidSet = gcnew XbimSolidSet();			
				XbimSolid^ body = XbimSolid::BuildClippingList(boolClip, clips);

				double maxLen = body->BoundingBox.Length();
				for each (IfcBooleanOperand^ bOp in clips)
				{
					IfcPolygonalBoundedHalfSpace^ pbhs = dynamic_cast<IfcPolygonalBoundedHalfSpace^>(bOp);
					if (pbhs != nullptr) //special case for IfcPolygonalBoundedHalfSpace to keep extrusion to the minimum
					{
						XbimSolid^ s = gcnew XbimSolid(pbhs, maxLen);
						if (s->IsValid) solidSet->Add(s);
					}
					else
					{
						XbimSolid^ s = gcnew XbimSolid(bOp);
						if (s->IsValid) solidSet->Add(s);
					}
				}
				//use a fuzzy tolerance of 1mm, less than this should be ignored			
				IXbimSolidSet^ xbimSolidSet = body->Cut(solidSet, mf->OneMilliMetre);
				if (xbimSolidSet != nullptr && xbimSolidSet->First != nullptr)
				{
					pSolid = new TopoDS_Solid(); 
					*pSolid = (XbimSolid^)(xbimSolidSet->First); //just take the first as that is what is intended by IFC schema				
				}
				return;
			}

#endif
			
			IfcBooleanOperand^ sOp = solid->SecondOperand;
			XbimSolid^ left = gcnew XbimSolid(fOp);
			XbimSolid^ right = gcnew XbimSolid(sOp);
			/*BRepTools::Write(left, "d:\\xbim\\l");
			BRepTools::Write(right, "d:\\xbim\\r");*/
			if (!left->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS006: IfcBooleanResult #{0} with invalid first operand", solid->EntityLabel);
				return;
			}

			pSolid = new TopoDS_Solid(); //make sure this is deleted if not used

			if (!right->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS007: IfcBooleanResult #{0} with invalid second operand", solid->EntityLabel);
				*pSolid = left; //return the left operand
				return;
			}

			
			IXbimGeometryObject^ result;
			try
			{
				
				switch (solid->Operator)
				{
				case IfcBooleanOperator::Union:
					result = left->Union(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Intersection:
					result = left->Intersection(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Difference:
					result = left->Cut(right, mf->PrecisionBoolean);
					break;
				}
			}
			catch (Exception^ xbimE)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES001: Error performing boolean operation for entity #{0}={1}\n{2}. The operation has been ignored", solid->EntityLabel, solid->GetType()->Name, xbimE->Message);
				*pSolid = left; //return the left operand
				return;
			}

			XbimSolidSet^ xbimSolidSet = dynamic_cast<XbimSolidSet^>(result);
#ifdef OCC_6_9_SUPPORTED //Later versions of OCC has fuzzy boolean which gives better results
			if (xbimSolidSet != nullptr && xbimSolidSet->First != nullptr)
			{
				*pSolid = (XbimSolid^)(xbimSolidSet->First); //just take the first as that is what is intended by IFC schema
			}
#else // otherwise we have to make sure we get a solid when an error occurs
			if (xbimSolidSet == nullptr || xbimSolidSet->First==nullptr)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES002: Error performing boolean operation for entity #{0}={1}. The operation has been ignored", solid->EntityLabel, solid->GetType()->Name);
				*pSolid = left; //return the left operand
			}
			else //Ifc requires just one solid as a result so just take the first
			{
				*pSolid = (XbimSolid^)(xbimSolidSet->First);
			}
#endif
		}

		void XbimSolid::Init(IfcBooleanOperand^ solid)
		{
			IfcSolidModel^ sol = dynamic_cast<IfcSolidModel^>(solid);
			if (sol != nullptr) return Init(sol);
			IfcHalfSpaceSolid^ hs = dynamic_cast<IfcHalfSpaceSolid^>(solid);
			if (hs != nullptr) return Init(hs,solid->ModelOf->ModelFactors->OneMetre*100); //take 100 metres as the largest extrusion
			IfcBooleanClippingResult^ bcr = dynamic_cast<IfcBooleanClippingResult^>(solid);
			if (bcr != nullptr) return Init(bcr);
			IfcBooleanResult^ br = dynamic_cast<IfcBooleanResult^>(solid);
			if (br != nullptr) //this should only return one solid
			{
				XbimSolidSet^ solids = gcnew XbimSolidSet(br);
				if (solids->First!=nullptr)
				pSolid = new TopoDS_Solid();
				XbimSolid^ solid  = (XbimSolid^)solids->First;
				*pSolid = solid;
				return;
			}
			IfcCsgPrimitive3D^ csg = dynamic_cast<IfcCsgPrimitive3D^>(solid);
			if (csg != nullptr) return Init(csg);
			throw gcnew NotImplementedException("Sub-Type of IfcBooleanOperand is not implemented");
		}


		void XbimSolid::Init(IfcCsgPrimitive3D^ ifcSolid)
		{
			IfcSphere^ sphere = dynamic_cast<IfcSphere^>(ifcSolid);
			if (sphere != nullptr) return Init(sphere);
			IfcBlock^ block = dynamic_cast<IfcBlock^>(ifcSolid);
			if (block != nullptr) return Init(block);
			IfcRightCircularCylinder^ cylinder = dynamic_cast<IfcRightCircularCylinder^>(ifcSolid);
			if (cylinder != nullptr) return Init(cylinder);
			IfcRightCircularCone^ cone = dynamic_cast<IfcRightCircularCone^>(ifcSolid);
			if (cone != nullptr) return Init(cone);
			throw gcnew NotImplementedException(String::Format("IfcCsgPrimitive3D of Type {0} in entity #{1} is not implemented", ifcSolid->GetType()->Name, ifcSolid->EntityLabel));
		}

		void XbimSolid::Init(IfcCsgSolid^ ifcSolid)
		{
			IfcCsgPrimitive3D^ csgPrim = dynamic_cast<IfcCsgPrimitive3D^>(ifcSolid->TreeRootExpression);
			if (csgPrim != nullptr) return Init(csgPrim);
			IfcBooleanResult^ booleanResult = dynamic_cast<IfcBooleanResult^>(ifcSolid->TreeRootExpression);
			if (booleanResult != nullptr) return Init(booleanResult);
			throw gcnew NotImplementedException(String::Format("IfcCsgSolid of Type {0} in entity #{1} is not implemented", ifcSolid->GetType()->Name, ifcSolid->EntityLabel));
		}

		void XbimSolid::Init(IfcSphere^ ifcSolid)
		{
			gp_Ax3 	gpax3 = XbimGeomPrim::ToAx3(ifcSolid->Position);			
			BRepPrimAPI_MakeSphere sphereMaker(gpax3.Ax2(), ifcSolid->Radius);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(sphereMaker.Shape());
		}

		void XbimSolid::Init(IfcBlock^ ifcSolid)
		{
			gp_Ax3 	gpax3 = XbimGeomPrim::ToAx3(ifcSolid->Position);
			BRepPrimAPI_MakeBox boxMaker(gpax3.Ax2(), ifcSolid->XLength, ifcSolid->YLength, ifcSolid->ZLength);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(boxMaker.Shape());
		}

		void XbimSolid::Init(IfcRightCircularCylinder^ ifcSolid)
		{
			gp_Ax3 	gpax3 = XbimGeomPrim::ToAx3(ifcSolid->Position);
			BRepPrimAPI_MakeCylinder cylinderMaker(gpax3.Ax2(), ifcSolid->Radius, ifcSolid->Height);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(cylinderMaker.Shape());
		}

		void XbimSolid::Init(IfcRightCircularCone^ ifcSolid)
		{
			gp_Ax3 	gpax3 = XbimGeomPrim::ToAx3(ifcSolid->Position);
			BRepPrimAPI_MakeCone coneMaker(gpax3.Ax2(), ifcSolid->BottomRadius, 0., ifcSolid->Height);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(coneMaker.Shape());
		}

		void XbimSolid::Init(IfcRectangularPyramid^ ifcSolid)
		{
			
			double xOff = ifcSolid->XLength / 2;
			double yOff = ifcSolid->YLength / 2;
			double precision = ifcSolid->ModelOf->ModelFactors->Precision;
			
			gp_Pnt bl(0, 0, 0);
			gp_Pnt br(ifcSolid->XLength, 0, 0);
			gp_Pnt tr(ifcSolid->XLength, ifcSolid->YLength, 0);
			gp_Pnt tl(0, ifcSolid->YLength, 0);
			gp_Pnt p(xOff, yOff, ifcSolid->Height);
			//make the vertices
			BRep_Builder builder;
			TopoDS_Vertex vbl, vbr, vtr, vtl, vp;
			TopoDS_Shell shell;
			builder.MakeShell(shell);
			builder.MakeVertex(vbl, bl, precision);
			builder.MakeVertex(vbr, br, precision);
			builder.MakeVertex(vtr, tr, precision);
			builder.MakeVertex(vtl, tl, precision);
			builder.MakeVertex(vp, p, precision);
			//make the edges
			TopoDS_Wire baseWire;
			builder.MakeWire(baseWire);
			const TopoDS_Edge& brbl = BRepBuilderAPI_MakeEdge(vbr, vbl);
			const TopoDS_Edge& trbr = BRepBuilderAPI_MakeEdge(vtr, vbr);
			const TopoDS_Edge& tltr = BRepBuilderAPI_MakeEdge(vtl, vtr);
			const TopoDS_Edge& bltl = BRepBuilderAPI_MakeEdge(vbl, vtl);
			builder.Add(baseWire, brbl);
			builder.Add(baseWire, bltl);
			builder.Add(baseWire, tltr);
			builder.Add(baseWire, trbr);
			BRepBuilderAPI_MakeFace afaceBlder(gp_Pln(gp_Pnt(0,0,0),gp_Dir(0,0,-1)),baseWire,Standard_True);
			builder.Add(shell, afaceBlder.Face());
			//build the sides
			const TopoDS_Edge& blp = BRepBuilderAPI_MakeEdge(vbl, vp);
			const TopoDS_Edge& tlp = BRepBuilderAPI_MakeEdge(vtl, vp);
			const TopoDS_Edge& brp = BRepBuilderAPI_MakeEdge(vbr, vp);
			const TopoDS_Edge& trp = BRepBuilderAPI_MakeEdge(vtr, vp);
			TopoDS_Wire bltlWire;
			builder.MakeWire(bltlWire);
			builder.Add(bltlWire, bltl.Reversed());
			builder.Add(bltlWire, blp); 
			builder.Add(bltlWire, tlp.Reversed());
			BRepBuilderAPI_MakeFace bfaceBlder(bltlWire, Standard_True);
			builder.Add(shell, bfaceBlder.Face());
			
			TopoDS_Wire tltrWire;
			builder.MakeWire(tltrWire);
			builder.Add(tltrWire, tltr.Reversed());
			builder.Add(tltrWire, tlp);
			builder.Add(tltrWire, trp.Reversed());
			BRepBuilderAPI_MakeFace cfaceBlder(tltrWire, Standard_True);
			builder.Add(shell, cfaceBlder.Face());


			TopoDS_Wire brtlWire;
			builder.MakeWire(brtlWire);
			builder.Add(brtlWire, trbr.Reversed());
			builder.Add(brtlWire, trp);
			builder.Add(brtlWire, brp.Reversed());
			BRepBuilderAPI_MakeFace dfaceBlder(brtlWire, Standard_True);
			builder.Add(shell, dfaceBlder.Face());

			TopoDS_Wire blbrWire;
			builder.MakeWire(blbrWire);
			builder.Add(blbrWire, brbl.Reversed());
			builder.Add(blbrWire, brp);
			builder.Add(blbrWire, blp.Reversed());
			BRepBuilderAPI_MakeFace efaceBlder(blbrWire, Standard_True);
			builder.Add(shell, efaceBlder.Face());

			BRepBuilderAPI_MakeSolid solidMaker(shell);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(solidMaker.Shape());
			Move(ifcSolid->Position);		
		}

		TopoDS_Face BuildTriangularFace(const TopoDS_Edge& base, const TopoDS_Vertex& l, const TopoDS_Vertex& r, const TopoDS_Vertex& t)
		{
			BRep_Builder builder;
			//build the sides
			const TopoDS_Edge& rt = BRepBuilderAPI_MakeEdge(r, t);
			const TopoDS_Edge& tl = BRepBuilderAPI_MakeEdge(t, l);
			TopoDS_Wire bltlWire;
			builder.MakeWire(bltlWire);
			builder.Add(bltlWire, base);
			builder.Add(bltlWire, rt);
			builder.Add(bltlWire, tl);
			BRepBuilderAPI_MakeFace faceBlder(bltlWire);
			return faceBlder.Face();
		}
#pragma endregion

#pragma region IXbimSolid Interface

		IXbimShellSet^ XbimSolid::Shells::get()
		{
			if (!IsValid) return XbimShellSet::Empty;
			return gcnew XbimShellSet(*pSolid);
		}


		IXbimFaceSet^ XbimSolid::Faces::get()
		{
			if (!IsValid) return XbimFaceSet::Empty;
			return gcnew XbimFaceSet(*pSolid);
		}
		
		IXbimEdgeSet^ XbimSolid::Edges::get()
		{
			if (!IsValid) return XbimEdgeSet::Empty;
			return gcnew XbimEdgeSet(*pSolid);
		}
		IXbimVertexSet^ XbimSolid::Vertices::get()
		{
			if (!IsValid) return XbimVertexSet::Empty;
			return gcnew XbimVertexSet(*pSolid);
		}

		XbimRect3D XbimSolid::BoundingBox::get()
		{
			if (pSolid == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			if (IsPolyhedron)
				BRepBndLib::AddClose(*pSolid, pBox);
			else
				BRepBndLib::Add(*pSolid, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		//returns true if the solid is a closed manifold typically with one shell, if there are more shells they are voids and should also be closed
		bool XbimSolid::IsClosed::get()
		{		
			for each (XbimShell^ shell in Shells)
				if (!shell->IsClosed) return false;
			return true;
		}

		double XbimSolid::Volume::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::VolumeProperties(*pSolid, gProps, Standard_True);
				GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;
		}

		bool XbimSolid::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			for (TopExp_Explorer exp(*pSolid, TopAbs_EDGE); exp.More(); exp.Next())
			{
				Standard_Real start, end;
				Handle(Geom_Curve) c3d = BRep_Tool::Curve(TopoDS::Edge(exp.Current()), start, end);
				if (!c3d.IsNull())
				{
					Handle(Standard_Type) cType = c3d->DynamicType();
					if (cType != STANDARD_TYPE(Geom_Line))
					{
						if (cType != STANDARD_TYPE(Geom_TrimmedCurve)) return false;
						Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c3d);
						Handle(Standard_Type) tcType = tc->DynamicType();
						if (tcType != STANDARD_TYPE(Geom_Line)) return false;
					}
				}
			}
			GC::KeepAlive(this);
			//all edges are lines
			return true;
		}
		double XbimSolid::SurfaceArea::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::SurfaceProperties(*pSolid, gProps);
				GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;
		}


		bool XbimSolid::HasValidTopology::get()
		{
			if (!IsValid) return false;
			BRepCheck_Analyzer analyser(*pSolid, Standard_True);
			GC::KeepAlive(this);
			return analyser.IsValid() == Standard_True;
		}

		IXbimGeometryObject^ XbimSolid::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Solid temp = TopoDS::Solid(gTran.Shape());
			return gcnew XbimSolid(temp);
		}

		IXbimGeometryObject^ XbimSolid::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Solid solid = TopoDS::Solid(pSolid->Moved(XbimGeomPrim::ToTransform(matrix3D)));
			GC::KeepAlive(this);
			return gcnew XbimSolid(solid);
		}

		IXbimSolidSet^ XbimSolid::Cut(IXbimSolidSet^ toCut, double tolerance)
		{
			if (toCut->Count == 0) return gcnew XbimSolidSet(this);
			if (toCut->Count == 1) return this->Cut(toCut->First, tolerance);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Cut(toCut, tolerance);
		}


		IXbimSolidSet^ XbimSolid::Cut(IXbimSolid^ toCut, double tolerance)
		{			
			if (!IsValid || !toCut->IsValid) return XbimSolidSet::Empty;
			XbimSolid^ solidCut = dynamic_cast<XbimSolid^>(toCut);
			if (solidCut == nullptr)
			{
#ifdef USE_CARVE_CSG
				XbimFacetedSolid^ facetedSolidCut = dynamic_cast<XbimFacetedSolid^>(toCut);
				if (facetedSolidCut != nullptr) //downgrade to facetation or upgrade and perform
				{
					if (this->IsPolyhedron) //downgrade this to facetation, faster
					{
						XbimFacetedSolid^ thisFacetedSolid = gcnew XbimFacetedSolid(this, tolerance);
						return thisFacetedSolid->Cut(facetedSolidCut, tolerance);
					}
					else //upgrade tocut to occ, more accurate with curves
					{
						solidCut = (XbimSolid^)facetedSolidCut->ConvertToXbimSolid();
						if (solidCut == nullptr)
						{
							XbimGeometryCreator::logger->WarnFormat("WS023: Invalid operation. Only solid shapes can be cut from another solid");
							return gcnew XbimSolidSet(this); // the result would be no change so return this
						} //else carry on with the boolean
					}
				}
				else
#endif // USE_CARVE_CSG

				{
					XbimGeometryCreator::logger->WarnFormat("WS024: Invalid operation. Only solid shapes can be cut from another solid");
					return gcnew XbimSolidSet(this); // the result would be no change so return this		
				}
			}

			
			String^ err="";
			try
			{
#ifdef OCC_6_9_SUPPORTED
				TopTools_ListOfShape shapeTools;
				shapeTools.Append(solidCut);
				TopTools_ListOfShape shapeObjects;
				shapeObjects.Append(this);
				BRepAlgoAPI_Cut boolOp;
				boolOp.SetArguments(shapeObjects);
				boolOp.SetTools(shapeTools);
				boolOp.SetFuzzyValue(tolerance);
				boolOp.Build();
#else
				ShapeFix_ShapeTolerance fixTol;
				fixTol.SetTolerance(solidCut, tolerance);
				fixTol.SetTolerance(this, tolerance);
				BRepAlgoAPI_Cut boolOp(this, solidCut);
#endif
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimSolidSet(boolOp.Shape());
				err = "Error = " + boolOp.ErrorStatus();
				
			}
			catch (Standard_Failure e)
			{
				 err = gcnew String(Standard_Failure::Caught()->GetMessageString());			
			}
			XbimGeometryCreator::logger->WarnFormat("WS029: Boolean Cut operation failed. " + err);
			GC::KeepAlive(solidCut);
			GC::KeepAlive(this);
			return XbimSolidSet::Empty;
		}

		IXbimSolidSet^ XbimSolid::Intersection(IXbimSolidSet^ toIntersect, double tolerance)
		{
			if (toIntersect->Count == 0) return gcnew XbimSolidSet(this);
			if (toIntersect->Count == 1) return this->Intersection(toIntersect->First, tolerance);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Intersection(toIntersect, tolerance);
		}

		IXbimSolidSet^ XbimSolid::Intersection(IXbimSolid^ toIntersect, double tolerance)
		{
			if (!IsValid || !toIntersect->IsValid) return XbimSolidSet::Empty;
			XbimSolid^ solidIntersect = dynamic_cast<XbimSolid^>(toIntersect);
			if (solidIntersect == nullptr)
			{
				
#ifdef USE_CARVE_CSG
				XbimFacetedSolid^ facetedSolidIntersect = dynamic_cast<XbimFacetedSolid^>(toIntersect);
				if (facetedSolidIntersect != nullptr) //downgrade to facetation or upgrade and perform
				{
					if (this->IsPolyhedron) //downgrade this to facetation, faster
					{
						XbimFacetedSolid^ thisFacetedSolid = gcnew XbimFacetedSolid(this, tolerance);
						return thisFacetedSolid->Cut(facetedSolidIntersect, tolerance);
					}
					else //upgrade tocut to occ, more accurate with curves
					{
						solidIntersect = (XbimSolid^)facetedSolidIntersect->ConvertToXbimSolid();
						if (solidIntersect == nullptr)
						{
							XbimGeometryCreator::logger->WarnFormat("WS025: Invalid operation. Only solid shapes can be intersected with another solid");
							return gcnew XbimSolidSet(this); // the result would be no change so return this
						} //else carry on with the boolean
					}
				}
				else
#endif // USE_CARVE_CSG

				{
					
					XbimGeometryCreator::logger->WarnFormat("WS026: Invalid operation. Only solid shapes can be intersected with another solid");
					return gcnew XbimSolidSet(this); // the result would be no change so return this
				}
			}
			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solidIntersect, tolerance);
			fixTol.SetTolerance(this, tolerance);
			String^ err = "";
			try
			{
				BRepAlgoAPI_Common boolOp(this, solidIntersect);
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimSolidSet(boolOp.Shape());
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::logger->WarnFormat("WS030: Boolean Intersect operation failed. " + err);
			return XbimSolidSet::Empty;
		}

		IXbimSolidSet^ XbimSolid::Union(IXbimSolidSet^ toUnion, double tolerance)
		{
			if (toUnion->Count == 0) return gcnew XbimSolidSet(this);
			if (toUnion->Count == 1) return this->Union(toUnion->First, tolerance);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Union(toUnion, tolerance);
		}

		IXbimSolidSet^ XbimSolid::Union(IXbimSolid^ toUnion, double tolerance)
		{
			if (!IsValid || !toUnion->IsValid) return XbimSolidSet::Empty;
			XbimSolid^ solidUnion = dynamic_cast<XbimSolid^>(toUnion);
			if (solidUnion == nullptr)
			{

#ifdef USE_CARVE_CSG
				XbimFacetedSolid^ facetedSolidUnion = dynamic_cast<XbimFacetedSolid^>(toUnion);
				if (facetedSolidUnion != nullptr) //downgrade to facetation or upgrade and perform
				{
					if (this->IsPolyhedron) //downgrade this to facetation, faster
					{
						XbimFacetedSolid^ thisFacetedSolid = gcnew XbimFacetedSolid(this, tolerance);
						return thisFacetedSolid->Cut(facetedSolidUnion, tolerance);
					}
					else //upgrade tocut to occ, more accurate with curves
					{
						solidUnion = (XbimSolid^)facetedSolidUnion->ConvertToXbimSolid();
						if (solidUnion == nullptr)
						{
							XbimGeometryCreator::logger->WarnFormat("WS027: Invalid operation. Only solid shapes can be unioned with another solid");
							return gcnew XbimSolidSet(this); // the result would be no change so return this
						} //else carry on with the boolean
					}
				}
				else
#endif // USE_CARVE_CSG

				{
					XbimGeometryCreator::logger->WarnFormat("WS028: Invalid operation. Only solid shapes can be unioned with another solid");
					return gcnew XbimSolidSet(this); // the result would be no change so return this
				}
			}
			
			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solidUnion, tolerance);
			fixTol.SetTolerance(this, tolerance);
			String^ err = "";
			try
			{
				BRepAlgoAPI_Fuse boolOp(this, solidUnion);
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimSolidSet(boolOp.Shape());
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::logger->WarnFormat("WS031: Boolean Union operation failed. " + err);
			return XbimSolidSet::Empty;
		}


		IXbimFaceSet^ XbimSolid::Section(IXbimFace^ toSection, double tolerance)
		{
			if (!IsValid || !toSection->IsValid) return XbimFaceSet::Empty;
			XbimFace^ faceSection = dynamic_cast<XbimFace^>(toSection);
			if (faceSection == nullptr)  throw gcnew ArgumentException("Only IXbimSolids created by Xbim.OCC modules are supported", "toSection");
			
			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(faceSection, tolerance);
			fixTol.SetTolerance(this, tolerance);
			BRepAlgoAPI_Section boolOp(this, faceSection, false);
			boolOp.ComputePCurveOn2(Standard_True);
			boolOp.Build();
			
			if (boolOp.IsDone())
			{
				Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape();
				Handle(TopTools_HSequenceOfShape) wires = new TopTools_HSequenceOfShape();
				for (TopExp_Explorer expl(boolOp.Shape(), TopAbs_EDGE); expl.More(); expl.Next())
					edges->Append(TopoDS::Edge(expl.Current()));
						
				TopoDS_Compound open;
				TopoDS_Compound closed;
				BRep_Builder b;
				b.MakeCompound(open);
				b.MakeCompound(closed);

				ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance, false, wires);
				ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
				TopExp_Explorer exp(closed, TopAbs_WIRE);
				if (!exp.More()) //try and resolve precision errors
				{
					wires->Clear();
					ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance * 10, false, wires);
					ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
					TopExp_Explorer exp2(closed, TopAbs_WIRE);
					if (!exp2.More())
					{
						wires->Clear();
						ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance * 100, false, wires);
						ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
					}
				}

				BRepAlgo_FaceRestrictor fr;
				TopoDS_Shape aLocalS = boolOp.Shape2().Oriented(TopAbs_FORWARD);
				fr.Init(TopoDS::Face(aLocalS), Standard_True, Standard_True);
				for (TopExp_Explorer exp(closed, TopAbs_WIRE); exp.More(); exp.Next()) 
				{
					ShapeFix_Wire wireFixer(TopoDS::Wire(exp.Current()), faceSection, tolerance);
					wireFixer.Perform();
					fr.Add(wireFixer.Wire());
				}
				fr.Perform();			
				if (fr.IsDone())
				{
					TopTools_ListOfShape result;
					TopAbs_Orientation orientationOfFace = boolOp.Shape2().Orientation();
					for (; fr.More(); fr.Next()) 
					{
						result.Append(TopoDS::Face(fr.Current().Oriented(orientationOfFace)));
					}
					
					return gcnew XbimFaceSet(result);
				}						
				GC::KeepAlive(faceSection);		
				GC::KeepAlive(toSection);
				GC::KeepAlive(this);
			}
			XbimGeometryCreator::logger->WarnFormat("WS008:Boolean Section operation has failed to create a section");
			
			return XbimFaceSet::Empty;
		}

			

#pragma endregion

#pragma region Boolean operations

	
		void XbimSolid::Move(IfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimGeomPrim::ToTransform(position);
			pSolid->Move(toPos);
		}

		void XbimSolid::Translate(XbimVector3D translation)
		{
			if (!IsValid) return;
			gp_Vec v(translation.X, translation.Y, translation.Z);
			gp_Trsf t;
			t.SetTranslation(v);
			pSolid->Move(t);
		}

		void XbimSolid::Reverse()
		{
			if (!IsValid) return;
			pSolid->Reverse();
		}

		
		
		void XbimSolid::FixTopology()
		{
			ShapeFix_Solid fixer(this);
			fixer.Perform();
			const TopoDS_Shape& fixed = fixer.Shape();
			if (fixed.ShapeType()==TopAbs_SHELL)
				*pSolid = fixer.SolidFromShell(TopoDS::Shell(fixed));
			else if(fixed.ShapeType()==TopAbs_SOLID)
				*pSolid = TopoDS::Solid(fixer.Shape());
		}


#pragma endregion
	}


}