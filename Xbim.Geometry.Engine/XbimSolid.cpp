#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"
#include "XbimWire.h"
#include "XbimWireSet.h"
#include "XbimCompound.h"
#include "XbimSolidSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimShellSet.h"
#include "XbimFaceSet.h"
#include "XbimPoint3DWithTolerance.h"

#include "XbimGeometryCreator.h"
#include "XbimConvert.h"
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
#include <BRepOffsetAPI_ThruSections.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgo_Loop.hxx>
#include <Geom_OffsetCurve.hxx>
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
#include <BRepBuilderAPI_GTransform.hxx>
#include <ShapeFix_Solid.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRepGProp_Face.hxx>
#include <BRepAlgo_Section.hxx>

using namespace System::Linq;
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

		XbimSolid::XbimSolid(const TopoDS_Solid& solid, Object^ tag) : XbimSolid(solid)
		{
			Tag = tag;
		}


		XbimSolid::XbimSolid(IIfcSolidModel^ solid, ILogger^ logger)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcManifoldSolidBrep^ solid, ILogger^ logger)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);
		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolid^ solid, ILogger^ logger)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolidTapered^ repItem, ILogger^ logger)
		{
			Init(repItem, nullptr, logger);

		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);

		}

		XbimSolid::XbimSolid(IIfcRevolvedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);

		}
		XbimSolid::XbimSolid(IIfcExtrudedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);

		}
		XbimSolid::XbimSolid(IIfcExtrudedAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, nullptr, logger);

		}

		XbimSolid::XbimSolid(IIfcExtrudedAreaSolidTapered^ repItem, ILogger^ logger)
		{
			Init(repItem, nullptr, logger);

		}
		XbimSolid::XbimSolid(IIfcExtrudedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);

		}
		XbimSolid::XbimSolid(IIfcSweptDiskSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}


		//Handled by IIfcSweptDiskSolid
		/*XbimSolid::XbimSolid(IIfcSweptDiskSolidPolygonal^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}*/

		XbimSolid::XbimSolid(IIfcSectionedSpine^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcBoundingBox^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}



		XbimSolid::XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, nullptr, logger);
		}

		XbimSolid::XbimSolid(IIfcSurfaceCurveSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);
		}

		XbimSolid::XbimSolid(IIfcHalfSpaceSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcPolygonalBoundedHalfSpace^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcBoxedHalfSpace^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(XbimRect3D rect3D, double tolerance, ILogger^ logger)
		{
			Init(rect3D, tolerance, logger);
		}

		XbimSolid::XbimSolid(IIfcTriangulatedFaceSet ^ IIfcSolid, ILogger ^ logger)
		{
			Init(IIfcSolid, logger);
		}

		XbimSolid::XbimSolid(IIfcFaceBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			Init(solid, logger);
		}

		XbimSolid::XbimSolid(IIfcShellBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			Init(solid, logger);
		}



		XbimSolid::XbimSolid(IIfcCsgPrimitive3D^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}



		XbimSolid::XbimSolid(IIfcSphere^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcBlock^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcRightCircularCylinder^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcRightCircularCone^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcRectangularPyramid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolid::XbimSolid(IIfcFixedReferenceSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, nullptr, logger);
		}

		XbimSolid::XbimSolid(IIfcFixedReferenceSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			Init(repItem, overrideProfileDef, logger);
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

		void XbimSolid::Init(IIfcSolidModel^ solid, ILogger^ logger)
		{
			IIfcSweptAreaSolid^ extrudeArea = dynamic_cast<IIfcSweptAreaSolid^>(solid);
			if (extrudeArea) return Init(extrudeArea, nullptr, logger);
			IIfcSweptDiskSolid^ sd = dynamic_cast<IIfcSweptDiskSolid^>(solid);
			if (sd != nullptr) return Init(sd, logger);
			IIfcManifoldSolidBrep^ ms = dynamic_cast<IIfcManifoldSolidBrep^>(solid);
			if (ms != nullptr)
				return Init(ms, logger);
			IIfcCsgSolid^ csg = dynamic_cast<IIfcCsgSolid^>(solid);
			if (csg != nullptr)
			{
				XbimGeometryCreator::LogError(logger, csg, "The Init method for IIfcSolidModel should not be called with an IIfcCsgSolid. Use XbimSolidSet");
				return Init(csg, logger);
			}
			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", solid->GetType()->Name, solid->EntityLabel));
		}

		void XbimSolid::Init(IIfcManifoldSolidBrep^ bRep, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(bRep, logger);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew();
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, bRep->Model->ModelFactors->Precision);
			}
		}


		void XbimSolid::Init(IIfcSweptAreaSolid^ solid, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			IIfcExtrudedAreaSolid^ extrudeArea = dynamic_cast<IIfcExtrudedAreaSolid^>(solid);
			if (extrudeArea) return Init(extrudeArea, overrideProfileDef, logger);
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(solid);
			if (ras != nullptr) return Init(ras, overrideProfileDef, logger);
			IIfcFixedReferenceSweptAreaSolid^ fas = dynamic_cast<IIfcFixedReferenceSweptAreaSolid^>(solid);
			if (fas != nullptr) return Init(fas, overrideProfileDef, logger);
			XbimGeometryCreator::LogError(logger, solid, "Swept Solid of Type {0} is not implemented", solid->GetType()->Name);
		}

		void XbimSolid::Init(IIfcSurfaceCurveSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger);
			if (!faceStart->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not build Swept Area");
				return;
			}

			IModelFactors^ mf = repItem->Model->ModelFactors;
			XbimWire^ sweep = gcnew XbimWire(repItem->Directrix, logger);

			if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
				sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, Math::Abs(repItem->EndParam.Value - 1.0) < Precision::Confusion() ? sweep->Length : repItem->EndParam.Value, mf->Precision, logger);
			else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
				sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, sweep->Length, mf->Precision, logger);
			else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
				sweep = (XbimWire^)sweep->Trim(0, Math::Abs(repItem->EndParam.Value - 1.0) < Precision::Confusion() ? sweep->Length : repItem->EndParam.Value, mf->Precision, logger);

			if (!sweep->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not build Directrix");
				return;
			}

			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);
			//find the start point of the sweep
			XbimPoint3D s = sweep->Start;
			gp_Pnt startPoint(s.X, s.Y, s.Z);
			//get where this is on the surface
			XbimFace^ refSurface = gcnew XbimFace(repItem->ReferenceSurface, logger);
			Handle(Geom_Surface) geomSurf = refSurface->GetSurface();
			GeomAPI_ProjectPointOnSurf projector(startPoint, geomSurf);
			projector.Perform(startPoint);
			Standard_Real u;
			Standard_Real v;
			projector.Parameters(1, u, v);
			XbimVector3D norm = refSurface->NormalAt(u, v);
			//move the wire to the start point
			TopoDS_Edge edge;
			Standard_Real uoe;
			BRepAdaptor_CompCurve cc(sweep);
			cc.Edge(0, edge, uoe);
			Standard_Real l, f;
			Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
			gp_Pnt p1;
			gp_Vec tangent;
			gp_Vec xDir;
			curve->D1(0, p1, tangent);
			gp_Ax3 toAx3(startPoint, tangent, gp_Vec(norm.X, norm.Y, norm.Z));	//rotate so normal of profile is tangental and X axis 
			gp_Trsf trsf;
			trsf.SetTransformation(toAx3, gp_Ax3());
			TopLoc_Location topLoc(trsf);
			faceStart->SetLocation(topLoc);
			XbimWire^ outerBound = (XbimWire^)(faceStart->OuterBound);

			BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
			pipeMaker1.SetTransitionMode(BRepBuilderAPI_RightCorner);
			pipeMaker1.Add(outerBound, Standard_False, Standard_False);
			pipeMaker1.Build();
			if (pipeMaker1.IsDone())
			{
				TopoDS_Wire firstOuter = TopoDS::Wire(pipeMaker1.FirstShape().Reversed());
				TopoDS_Wire lastOuter = TopoDS::Wire(pipeMaker1.LastShape().Reversed());
				BRepBuilderAPI_MakeFace firstMaker(firstOuter);
				BRepBuilderAPI_MakeFace lastMaker(lastOuter);
				for (int i = 0; i < faceStart->InnerBounds->Count; i++)
				{
					//it is a hollow section so we need to build the inside
					BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
					TopoDS_Wire innerBoundStart = faceStart->InnerWires->Wire[i];
					pipeMaker2.SetTransitionMode(BRepBuilderAPI_RightCorner);
					pipeMaker2.Add(innerBoundStart);
					pipeMaker2.Build();
					if (pipeMaker2.IsDone())
					{
						for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
						{
							b.AddShellFace(shell, TopoDS::Face(explr.Current().Reversed()));
						}
					}
					firstMaker.Add(TopoDS::Wire(pipeMaker2.FirstShape()));
					lastMaker.Add(TopoDS::Wire(pipeMaker2.LastShape()));
				}
				b.AddShellFace(shell, firstMaker.Face());
				b.AddShellFace(shell, lastMaker.Face());
				for (TopExp_Explorer explr(pipeMaker1.Shape(), TopAbs_FACE); explr.More(); explr.Next())
				{
					b.AddShellFace(shell, TopoDS::Face(explr.Current()));
				}
				TopoDS_Solid solid;
				BRep_Builder bs;
				bs.MakeSolid(solid);
				bs.Add(solid, shell);
				BRepClass3d_SolidClassifier sc(solid);
				sc.PerformInfinitePoint(Precision::Confusion());
				if (sc.State() == TopAbs_IN)
				{
					bs.MakeSolid(solid);
					shell.Reverse();
					bs.Add(solid, shell);

				}
				pSolid = new TopoDS_Solid();
				if (BRepCheck_Analyzer(solid, Standard_False).IsValid() == Standard_False)
				{
					ShapeFix_Shape shapeFixer(solid);
					shapeFixer.SetPrecision(mf->Precision);
					shapeFixer.SetMinTolerance(mf->Precision);
					shapeFixer.FixFaceTool()->FixIntersectingWiresMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixOrientationMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixWireTool()->FixAddCurve3dMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixWireTool()->FixIntersectingEdgesMode() = Standard_True;
					if (shapeFixer.Perform())
					{
						TopoDS_Shell sshell;
						b.MakeShell(sshell);
						for (TopExp_Explorer explr(shapeFixer.Shape(), TopAbs_FACE); explr.More(); explr.Next())
						{
							b.AddShellFace(sshell, TopoDS::Face(explr.Current()));
						}
						bs.MakeSolid(*pSolid);
						bs.Add(*pSolid, sshell);
					}
					else
						*pSolid = solid;
				}
				else
					*pSolid = solid;

				pSolid->Closed(Standard_True);
				if (repItem->Position != nullptr) //In Ifc4 this is now optional
					pSolid->Move(XbimConvert::ToLocation(repItem->Position));
				return;
			}

			GC::KeepAlive(faceStart);
			//GC::KeepAlive(faceEnd);


		}
		void XbimSolid::Init(IIfcRevolvedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger);
			XbimFace^ faceEnd = gcnew XbimFace(repItem->EndSweptArea, logger);

			if (faceStart->IsValid && faceEnd->IsValid && repItem->Angle > 0) //we have a valid face and angle
			{
				IIfcAxis1Placement^ revolaxis = repItem->Axis;
				gp_Pnt origin(revolaxis->Location->X, revolaxis->Location->Y, revolaxis->Location->Z);
				XbimVector3D zDir = repItem->Axis->Z;
				gp_Dir vz(zDir.X, zDir.Y, zDir.Z);

				double radianConvert = repItem->Model->ModelFactors->AngleToRadiansConversionFactor;
				//create a curve trimmed to the 
				XbimPoint3D faceCentre = faceStart->Location;
				XbimPoint3D rotCentre(origin.X(), origin.Y(), origin.Z());
				XbimVector3D v = faceCentre - rotCentre;
				gp_Ax2 ax2(origin, vz, gp_Vec(v.X, v.Y, v.Z));
				gp_Circ circ(ax2, v.Length);
				double angle = Math::Min(repItem->Angle*radianConvert, M_PI * 2);;
				GC_MakeArcOfCircle arcMaker(circ, 0., angle, Standard_True);
				Handle(Geom_TrimmedCurve) trimmed = arcMaker.Value();
				XbimCurve^ curve = gcnew XbimCurve(trimmed);
				XbimEdge^ edge = gcnew XbimEdge(curve);
				XbimWire^ sweep = gcnew XbimWire(edge);
				//move the end face to the end position
				gp_Pnt ep; gp_Vec tan; gp_Vec norm;
				trimmed->D2(trimmed->LastParameter(), ep, tan, norm);
				gp_Ax3 toAx3(ep, tan, norm);	//rotate so normal of profile is tangental and X axis 
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				faceEnd->Move(trsf);

				try
				{


					BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
					TopoDS_Wire outerBoundStart = (XbimWire^)(faceStart->OuterBound);
					TopoDS_Wire outerBoundEnd = (XbimWire^)(faceEnd->OuterBound);

					//pipeMaker1.SetMode(Standard_True);

					// todo: all usages of BRepBuilderAPI_Transformed should be reviewed
					// it's possible that BRepBuilderAPI_RightCorner should be used in most cases
					// 
					pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
					pipeMaker1.Add(outerBoundStart);
					pipeMaker1.Add(outerBoundEnd);
					pipeMaker1.Build();
					if (pipeMaker1.IsDone())
					{

						TopoDS_Wire firstOuter = TopoDS::Wire(pipeMaker1.FirstShape().Reversed());
						TopoDS_Wire lastOuter = TopoDS::Wire(pipeMaker1.LastShape().Reversed());
						BRepBuilderAPI_MakeFace firstMaker(firstOuter);
						BRepBuilderAPI_MakeFace lastMaker(lastOuter);
						for (int i = 0; i < faceStart->InnerBounds->Count; i++)
						{
							//it is a hollow section so we need to build the inside
							BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
							TopoDS_Wire innerBoundStart = faceStart->InnerWires->Wire[i];
							TopoDS_Wire innerBoundEnd = faceEnd->InnerWires->Wire[i];

							pipeMaker2.SetTransitionMode(BRepBuilderAPI_Transformed);
							pipeMaker2.Add(innerBoundStart);
							pipeMaker2.Add(innerBoundEnd);
							pipeMaker2.Build();
							if (pipeMaker2.IsDone())
							{
								for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
								{
									b.AddShellFace(shell, TopoDS::Face(explr.Current().Reversed()));
								}
							}
							firstMaker.Add(TopoDS::Wire(pipeMaker2.FirstShape()));
							lastMaker.Add(TopoDS::Wire(pipeMaker2.LastShape()));
						}
						b.AddShellFace(shell, firstMaker.Face());
						b.AddShellFace(shell, lastMaker.Face());
						for (TopExp_Explorer explr(pipeMaker1.Shape(), TopAbs_FACE); explr.More(); explr.Next())
						{
							b.AddShellFace(shell, TopoDS::Face(explr.Current()));
						}
						TopoDS_Solid solid;
						BRep_Builder bs;
						bs.MakeSolid(solid);
						bs.Add(solid, shell);
						BRepClass3d_SolidClassifier sc(solid);
						sc.PerformInfinitePoint(Precision::Confusion());
						if (sc.State() == TopAbs_IN)
						{
							bs.MakeSolid(solid);
							shell.Reverse();
							bs.Add(solid, shell);

						}
						pSolid = new TopoDS_Solid();
						*pSolid = solid;
						pSolid->Closed(Standard_True);
						if (repItem->Position != nullptr) //In Ifc4 this is now optional
							pSolid->Move(XbimConvert::ToLocation(repItem->Position));
						ShapeFix_ShapeTolerance tolFixer;
						tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
						return;
					}
					else if (repItem->Angle <= 0)
					{
						XbimGeometryCreator::LogInfo(logger, repItem, "Invalid extrusion,  angle must be >0 ");
					}
				}
				catch (Standard_Failure sf)
				{
					String^ err = gcnew String(sf.GetMessageString());
					XbimGeometryCreator::LogWarning(logger, repItem, "Failed to create  IfcRevolvedAreaSolidTapered solid: " + err);
				}
				catch (...)
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Failed to create  IfcRevolvedAreaSolidTapered solid");
				}
				GC::KeepAlive(faceStart);
				GC::KeepAlive(faceEnd);

			}
		}

		void XbimSolid::Init(IIfcExtrudedAreaSolidTapered^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger);
			XbimFace^ faceEnd = gcnew XbimFace(repItem->EndSweptArea, logger);

			if (faceStart->IsValid && faceEnd->IsValid && repItem->Depth > 0) //we have valid faces and extrusion
			{

				double precision = repItem->Model->ModelFactors->Precision;
				IIfcDirection^ dir = repItem->ExtrudedDirection;
				XbimVector3D vec(dir->X, dir->Y, dir->Z);
				vec = vec.Normalized();
				vec *= repItem->Depth;
				faceEnd->Translate(vec);
				try
				{
					BRepOffsetAPI_ThruSections pipeMaker(Standard_True, Standard_True, precision);
					TopoDS_Wire outerBoundStart = (XbimWire^)(faceStart->OuterBound);
					TopoDS_Wire outerBoundEnd = (XbimWire^)(faceEnd->OuterBound);
					pipeMaker.AddWire(outerBoundStart);
					pipeMaker.AddWire(outerBoundEnd);
					for (int i = 0; i < faceStart->InnerBounds->Count; i++)
					{
						TopoDS_Wire innerBoundStart = faceStart->InnerWires->Wire[i];
						faceEnd->InnerWires->Wire[i]->Translate(vec);
						TopoDS_Wire innerBoundEnd = faceEnd->InnerWires->Wire[i];
						pipeMaker.AddWire(innerBoundStart);
						pipeMaker.AddWire(innerBoundEnd);
					}
					pipeMaker.Build();
					if (pipeMaker.IsDone() && pipeMaker.Shape().ShapeType() == TopAbs_ShapeEnum::TopAbs_SOLID)
					{
						pSolid = new TopoDS_Solid();
						*pSolid = TopoDS::Solid(pipeMaker.Shape());
						pSolid->Closed(Standard_True);
						if (repItem->Position != nullptr) //In Ifc4 this is now optional
							pSolid->Move(XbimConvert::ToLocation(repItem->Position));
						ShapeFix_ShapeTolerance tolFixer;
						tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
						return;
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion, not a solid ");
					}
				}
				catch (Standard_Failure ex)
				{
					String^ err = gcnew String(ex.GetMessageString());
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion: " + err);
				}
				catch (...) //catch the access exceptions if raised and return gracefully
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion - ignored");
				}

				GC::KeepAlive(faceStart);
				GC::KeepAlive(faceEnd);
			}
			XbimGeometryCreator::LogWarning(logger, repItem, "Invalid tapered extrusion, depth must be >0 and faces must be correctly defined");
		}

		void XbimSolid::Init(IIfcFixedReferenceSweptAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);
			XbimFace^ faceStart;
			if (overrideProfileDef == nullptr)
				faceStart = gcnew XbimFace(repItem->SweptArea, logger);
			else
				faceStart = gcnew XbimFace(overrideProfileDef, logger);
			if (!faceStart->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not build swept area");
				return;
			}

			IModelFactors^ mf = repItem->Model->ModelFactors;
			XbimWire^ sweep = gcnew XbimWire(repItem->Directrix, logger);

			if (dynamic_cast<IIfcLine^>(repItem->Directrix)) //params are different
			{
				IIfcLine ^  line = (IIfcLine^)(repItem->Directrix);
				double mag = line->Dir->Magnitude;
				if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value *mag, repItem->EndParam.Value * mag, mf->Precision, logger);
				else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value*mag, sweep->Length, mf->Precision, logger);
				else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(0, repItem->EndParam.Value * mag, mf->Precision, logger);
			}
			else
			{
				if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, repItem->EndParam.Value, mf->Precision, logger);
				else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, sweep->Length, mf->Precision, logger);
				else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(0, repItem->EndParam.Value, mf->Precision, logger);
			}
			if (!sweep->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not build directrix");
				return;
			}
			if (faceStart->IsValid) //we have valid faces and extrusion
			{

				IIfcDirection^ xdir = repItem->FixedReference;
				gp_Vec xVec(xdir->X, xdir->Y, XbimConvert::GetZValueOrZero(xdir));
				xVec.Normalize();

				TopoDS_Edge edge;
				Standard_Real uoe;
				BRepAdaptor_CompCurve cc(sweep);
				cc.Edge(0, edge, uoe);
				Standard_Real l, f;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
				//move the wire to the start point
				gp_Pnt p1;
				gp_Vec tangent;
				curve->D1(0, p1, tangent);

				gp_Ax3 toAx3(p1, tangent, xVec);	//rotate so normal of profile is tangental and X axis 
				gp_Trsf trsf;
				trsf.SetTransformation(toAx3, gp_Ax3());
				TopLoc_Location topLoc(trsf);
				faceStart->SetLocation(topLoc);
				XbimWire^ outerBound = (XbimWire^)(faceStart->OuterBound);
				BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
				//pipeMaker1.SetMode(Standard_True);
				pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
				pipeMaker1.Add(outerBound);
				pipeMaker1.Build();
				if (pipeMaker1.IsDone())
				{
					TopoDS_Wire firstOuter = TopoDS::Wire(pipeMaker1.FirstShape());
					TopoDS_Wire lastOuter = TopoDS::Wire(pipeMaker1.LastShape().Reversed());

					BRepBuilderAPI_MakeFace firstMaker(firstOuter);
					BRepBuilderAPI_MakeFace lastMaker(lastOuter);
					for (int i = 0; i < faceStart->InnerBounds->Count; i++)
					{

						//it is a hollow section so we need to build the inside
						BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
						TopoDS_Wire innerBoundStart = faceStart->InnerWires->Wire[i];
						//pipeMaker2.SetMode(Standard_True);
						pipeMaker2.SetTransitionMode(BRepBuilderAPI_Transformed);
						pipeMaker2.Add(innerBoundStart);
						pipeMaker2.Build();
						if (pipeMaker2.IsDone())
						{
							for (TopExp_Explorer explr(pipeMaker2.Shape(), TopAbs_FACE); explr.More(); explr.Next())
							{
								b.AddShellFace(shell, TopoDS::Face(explr.Current()));
							}
						}
						firstMaker.Add(TopoDS::Wire(pipeMaker2.FirstShape()));
						lastMaker.Add(TopoDS::Wire(pipeMaker2.LastShape().Reversed()));
					}
					b.AddShellFace(shell, firstMaker.Face());
					b.AddShellFace(shell, lastMaker.Face());

					for (TopExp_Explorer explr(pipeMaker1.Shape(), TopAbs_FACE); explr.More(); explr.Next())
					{
						b.AddShellFace(shell, TopoDS::Face(explr.Current()));
					}
					b.CompleteShell(shell);
					ShapeFix_Shell sf(shell);
					if (sf.FixFaceOrientation(shell))
					{
						shell = sf.Shell();
					}
					TopoDS_Solid solid;
					BRep_Builder bs;
					bs.MakeSolid(solid);
					bs.Add(solid, shell);
					BRepClass3d_SolidClassifier sc(solid);
					sc.PerformInfinitePoint(Precision::Confusion());
					if (sc.State() == TopAbs_IN)
					{
						bs.MakeSolid(solid);
						shell.Reverse();
						bs.Add(solid, shell);

					}
					pSolid = new TopoDS_Solid();
					*pSolid = solid;
					pSolid->Closed(Standard_True);
					if (repItem->Position != nullptr) //In Ifc4 this is now optional
						pSolid->Move(XbimConvert::ToLocation(repItem->Position));
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
					return;
				}
				GC::KeepAlive(faceStart);

			}
			XbimGeometryCreator::LogInfo(logger, repItem, "Invalid extrusion, depth must be >0 and faces must be correctly defined");
			//if it has failed we will have a null solid

		}

		void XbimSolid::Init(IIfcSectionedSpine^ repItem, ILogger^ logger)
		{
			BRepPrim_Builder b;
			TopoDS_Shell shell;
			b.MakeShell(shell);

			//make a list of faces for each cross section (wire is not good enough as there ay be holes to consider
			List<XbimFace^>^ crossSections = gcnew List<XbimFace^>();
			for each (IIfcProfileDef^ profile in repItem->CrossSections)
			{
				XbimFace^ f = gcnew XbimFace(profile, logger);
				crossSections->Add(f);
			}
			List<IIfcAxis2Placement3D^>^ positions = Enumerable::ToList<IIfcAxis2Placement3D^>(repItem->CrossSectionPositions);

			if (crossSections->Count > 1) //we have valid faces 
			{

				//build the spine
				XbimWire^ sweep = gcnew XbimWire(repItem->SpineCurve, logger);
				BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
				pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
				//move the sections to the right position
				for (int i = 0; i < crossSections->Count; i++)
				{
					crossSections[i]->Move(positions[i]);
					TopoDS_Wire outerBound = (XbimWire^)(crossSections[i]->OuterBound);
					pipeMaker1.Add(outerBound);
				}
				bool ok = false;
				try
				{
					pipeMaker1.Build();
					ok = true;
				}
				catch (...)
				{
				}
				if (!ok)
				{
					BRepBuilderAPI_PipeError err = pipeMaker1.GetStatus();
					XbimGeometryCreator::LogError(logger, repItem, "Failed to build outer shell of sectioned spine {0}", (int)err);
					return;
				}
				if (pipeMaker1.IsDone())
				{
					if (!pipeMaker1.MakeSolid())
					{
						XbimGeometryCreator::LogWarning(logger, repItem, "Could not construct IfcSectionedSpine");
						return;
					}
					TopoDS_Solid outerShell = TopoDS::Solid(pipeMaker1.Shape());

					for (int i = 0; i < crossSections[0]->InnerBounds->Count; i++) //assume all sections have same topology
					{
						//it is a hollow section so we need to build the inside
						BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
						pipeMaker2.SetTransitionMode(BRepBuilderAPI_Transformed);
						for (int j = 0; j < crossSections->Count; j++)
						{
							TopoDS_Wire innerBound = (XbimWire^)(crossSections[j]->InnerWires->Wire[i]);
							pipeMaker2.Add(innerBound);
						}
						ok = false;
						try
						{
							pipeMaker2.Build();
							ok = true;
						}
						catch (...)
						{

						}
						if (!ok)
						{
							BRepBuilderAPI_PipeError err = pipeMaker2.GetStatus();
							XbimGeometryCreator::LogError(logger, repItem, "Failed to build inner shell of sectioned spine {0}", (int)err);
							return;
						}
						if (pipeMaker2.IsDone())
						{
							if (!pipeMaker2.MakeSolid()) //we cannot make a solid
							{
								XbimGeometryCreator::LogWarning(logger, repItem, "Could not construct inner sweep of IfcSectionedSpine");
							}
							else
							{
								TopoDS_Solid innerShell = TopoDS::Solid(pipeMaker2.Shape());
								TopTools_IndexedMapOfShape outerShellMap;
								TopExp::MapShapes(outerShell, TopAbs_FACE, outerShellMap);
								TopoDS_Face firstFace;
								TopoDS_Face lastFace;
								for (int k = 1; k <= outerShellMap.Extent(); k++)
								{
									const TopoDS_Face & f = TopoDS::Face(outerShellMap(k));
									if (f.IsSame(pipeMaker1.FirstShape()))
										firstFace = f;
									else if (f.IsSame(pipeMaker1.LastShape()))
										lastFace = f;
									else
										b.AddShellFace(shell, f);
								}
								TopTools_IndexedMapOfShape innerShellMap;
								TopExp::MapShapes(innerShell, TopAbs_FACE, innerShellMap);
								for (int k = 1; k <= innerShellMap.Extent(); k++)
								{
									const TopoDS_Face & f = TopoDS::Face(innerShellMap(k));

									if (f.IsEqual(pipeMaker2.FirstShape()))
									{
										BRepBuilderAPI_MakeFace firstMaker(firstFace);
										TopoDS_Wire w = BRepTools::OuterWire(f);
										w.Reverse();
										firstMaker.Add(w);
										firstFace = firstMaker.Face();
									}
									else if (f.IsEqual(pipeMaker2.LastShape()))
									{
										BRepBuilderAPI_MakeFace lastMaker(lastFace);
										TopoDS_Wire w = BRepTools::OuterWire(f);
										w.Reverse();
										lastMaker.Add(w);
										lastFace = lastMaker.Face();
									}
									else
										b.AddShellFace(shell, TopoDS::Face(f.Reversed()));

								}
								TopoDS_Solid solid;
								BRep_Builder bs;
								bs.MakeSolid(solid);

								bs.Add(shell, firstFace);
								bs.Add(shell, lastFace);
								bs.Add(solid, shell);

								BRepClass3d_SolidClassifier SC(solid);
								SC.PerformInfinitePoint(Precision::Confusion());
								if (SC.State() == TopAbs_IN) {
									bs.MakeSolid(solid);
									shell.Reverse();
									bs.Add(solid, shell);
								}
								solid.Closed(Standard_True);
								outerShell = solid;
							}
						}
						else
						{
							XbimGeometryCreator::LogWarning(logger, repItem, "Inner loop of IfcSectionedSpine could not be constructed");
						}

					}

					pSolid = new TopoDS_Solid();
					*pSolid = outerShell;
					pSolid->Closed(Standard_True);
					GC::KeepAlive(crossSections);
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
					return;
				}
				else
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "IfcSectionedSpine ould not be constructed");

				}
			}
			XbimGeometryCreator::LogInfo(logger, repItem, "Invalid IfcSectionedSpine, sections must be 2 or more and faces must be correctly defined");
			//if it has failed we will have a null solid
		}



		void XbimSolid::Init(IIfcExtrudedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			IIfcExtrudedAreaSolidTapered^ extrudeTaperedArea = dynamic_cast<IIfcExtrudedAreaSolidTapered^>(repItem);
			if (extrudeTaperedArea != nullptr) return Init(extrudeTaperedArea, overrideProfileDef, logger);
			IIfcCompositeProfileDef^ compProf = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);


			IIfcDirection^ dir = repItem->ExtrudedDirection;
			gp_Vec vec(dir->X, dir->Y, dir->Z);
			vec.Normalize();
			double depth = repItem->Depth;
			if (depth > 1e36) //SRL 1e36 is as big as we can go without booleans failing, it should be big enough for most sensible cases
			{
				XbimGeometryCreator::LogInfo(logger, repItem, "Extrusion is too large, it has been truncated to avoid boolean errors");
				depth = 1e36;
			}

			vec *= depth;

			if (repItem->Depth > 0) //we have a valid face and extrusion
			{
				if (compProf != nullptr && compProf->Profiles->Count > 1 && overrideProfileDef == nullptr)
				{
					XbimGeometryCreator::LogError(logger, repItem, "Composite profiles with more than 1 profile cannot create a solid, use the CreateSolidSet method");
				}
				else if (compProf != nullptr && compProf->Profiles->Count == 1) overrideProfileDef = compProf->Profiles->GetAt(0);

				XbimFace^ face;
				if (overrideProfileDef == nullptr)
					face = gcnew XbimFace(repItem->SweptArea, logger);
				else
					face = gcnew XbimFace(overrideProfileDef, logger);
				if (!face->BoundingBox.IsEmpty)
				{

					BRepPrimAPI_MakePrism prism(face, vec);
					GC::KeepAlive(face);
					if (prism.IsDone())
					{
						pSolid = new TopoDS_Solid();
						*pSolid = TopoDS::Solid(prism.Shape());
						if (repItem->Position != nullptr) //In Ifc4 this is now optional
							pSolid->Move(XbimConvert::ToLocation(repItem->Position));
						ShapeFix_ShapeTolerance tolFixer;
						tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
						return;
					}
				}
				// XbimGeometryCreator::LogWarning(logger, repItem, "Invalid extrusion, could not create solid");

			}
			else if (repItem->Depth <= 0)
			{
				XbimGeometryCreator::LogInfo(logger, repItem, "Invalid extrusion, depth must be >0");
			}
			//if it has failed we will have a null solid
		}

		void XbimSolid::Init(IIfcRevolvedAreaSolid^ repItem, IIfcProfileDef^ overrideProfileDef, ILogger^ logger)
		{
			IIfcRevolvedAreaSolidTapered^ extrudeTaperedArea = dynamic_cast<IIfcRevolvedAreaSolidTapered^>(repItem);
			if (extrudeTaperedArea != nullptr) return Init(extrudeTaperedArea, overrideProfileDef, logger);
			XbimFace^ face;
			if (overrideProfileDef == nullptr)
				face = gcnew XbimFace(repItem->SweptArea, logger);
			else
				face = gcnew XbimFace(overrideProfileDef, logger);

			if (face->IsValid && repItem->Angle > 0) //we have a valid face and angle
			{
				IIfcAxis1Placement^ revolaxis = repItem->Axis;
				gp_Pnt origin(revolaxis->Location->X, revolaxis->Location->Y, revolaxis->Location->Z);
				XbimVector3D zDir = revolaxis->Z;
				gp_Dir vx(zDir.X, zDir.Y, zDir.Z);
				gp_Ax1 ax1(origin, vx);
				double radianConvert = repItem->Model->ModelFactors->AngleToRadiansConversionFactor;
				BRepPrimAPI_MakeRevol revol(face, ax1, repItem->Angle*radianConvert);

				GC::KeepAlive(face);
				if (revol.IsDone())
				{
					//BRepTools::Write(revol.Shape(), "d:\\tmp\\rev");
					pSolid = new TopoDS_Solid();
					if (repItem->Position != nullptr)
						*pSolid = TopoDS::Solid(revol.Shape());
					pSolid->Move(XbimConvert::ToLocation(repItem->Position));
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
				}
				else
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalidextrusion, could not create solid");
			}
			else if (repItem->Angle <= 0)
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Invalidextrusion, angle must be >0");
			}
		}

		void XbimSolid::Init(IIfcHalfSpaceSolid^ hs, ILogger^ logger)
		{
			if (dynamic_cast<IIfcPolygonalBoundedHalfSpace^>(hs))
				Init((IIfcPolygonalBoundedHalfSpace^)hs, logger);
			else if (dynamic_cast<IIfcBoxedHalfSpace^>(hs))
				Init((IIfcBoxedHalfSpace^)hs, logger);
			else //it is a simple Half space
			{

				IIfcSurface^ surface = (IIfcSurface^)hs->BaseSurface;
				XbimFace^ face = gcnew XbimFace(surface, logger);

				BRepGProp_Face prop(face);
				gp_Pnt c;
				gp_Vec normalDir;
				double u1, u2, v1, v2;
				prop.Bounds(u1, u2, v1, v2);
				prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, c, normalDir);
				if (hs->AgreementFlag)
					normalDir.Reverse();
				gp_Pnt pointInMaterial = c.Translated(normalDir);
				BRepPrimAPI_MakeHalfSpace hsMaker(face, pointInMaterial);
				pSolid = new TopoDS_Solid();
				*pSolid = hsMaker.Solid();

				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, hs->Model->ModelFactors->Precision);
			}
		}

		void XbimSolid::Init(XbimRect3D rect3D, double tolerance, ILogger^ /*logger*/)
		{

			XbimPoint3D l = rect3D.Location;
			BRepPrimAPI_MakeBox box(gp_Pnt(l.X, l.Y, l.Z), rect3D.SizeX, rect3D.SizeY, rect3D.SizeZ);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(box.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, tolerance);
		}

		void XbimSolid::Init(IIfcBoxedHalfSpace^ bhs, ILogger^ logger)
		{
			IIfcSurface^ surface = (IIfcSurface^)bhs->BaseSurface;
			if (!dynamic_cast<IIfcPlane^>(surface))
			{
				XbimGeometryCreator::LogWarning(logger, bhs, "Non-Planar half spaces are not supported. It has been ignored");
				return;
			}
			IIfcPlane^ ifcPlane = (IIfcPlane^)surface;

			Init(bhs->Enclosure, logger);
			if (bhs->AgreementFlag)
				Translate(XbimVector3D(0, 0, -bhs->Enclosure->ZDim));
			Move(ifcPlane->Position);
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, bhs->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcPolygonalBoundedHalfSpace^ pbhs, ILogger^ logger)
		{

			//build the half space
			IIfcPlane^ surface = dynamic_cast<IIfcPlane^>(pbhs->BaseSurface);
			if (surface == nullptr)
			{
				XbimGeometryCreator::LogWarning(logger, pbhs, "Polygonal bounded of half space must have a planar surface. It has been ignored");
				return;
			}
			//get the surface of the half space
			gp_Ax3 ax3 = XbimConvert::ToAx3(surface->Position);
			gp_Pln pln(ax3);
			
			//build the substraction body
			gp_Dir dir(pbhs->Position->P[2].X, pbhs->Position->P[2].Y, pbhs->Position->P[2].Z);
			XbimWire^ polyBoundary = gcnew XbimWire(pbhs->PolygonalBoundary, logger);

			if (!polyBoundary->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, pbhs, "Polygonal boundary of half space is invalid or empty. It has been ignored");
				return;
			}

			//just simple check to see if the wire is sensible
			XbimVector3D normal = polyBoundary->Normal;
			if (normal.IsInvalid()) //check we have a valid wire
			{
				XbimGeometryCreator::LogWarning(logger, pbhs, "Polygonal boundary of half space is not an area. It has been ignored");
				return;
			}
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(polyBoundary, pbhs->Model->ModelFactors->Precision);
			// we have to use 1e8 as the max extrusion as the common boolean op times out on values greater than this, probably an extrema issue in booleans
			TopoDS_Shape substractionBody = BRepPrimAPI_MakePrism(BRepBuilderAPI_MakeFace(polyBoundary), gp_Vec(0, 0, 1e8));
			//find point inside the material
			gp_Pnt pnt = pln.Location().Translated(pbhs->AgreementFlag ? -pln.Axis().Direction() : pln.Axis().Direction());

			TopoDS_Shape halfspace = BRepPrimAPI_MakeHalfSpace(BRepBuilderAPI_MakeFace(pln), pnt).Solid();
			gp_Trsf location = XbimConvert::ToTransform(pbhs->Position);
			gp_Trsf shift;
			shift.SetTranslation(gp_Vec(0, 0, -1e8/2));
			substractionBody.Move(location*shift);
			TopoDS_Shape bounded_halfspace = BRepAlgoAPI_Common(substractionBody, halfspace);			 
			
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(bounded_halfspace, TopAbs_SOLID, map);
			if (map.Extent() != 1)
			{
				XbimGeometryCreator::LogWarning(logger, pbhs, "A single solid should be created for a Polygonal boundary of half space. It has been ignored");
				return;
			}
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(map(1));
			tolFixer.LimitTolerance(*pSolid, pbhs->Model->ModelFactors->Precision);
		}

		// params depend on segment type
		double XbimSolid::SegLenght(IIfcCompositeCurveSegment^ segment)
		{
			if (dynamic_cast<IIfcLine^>(segment->ParentCurve)) 
			{
				return 1;
			}
			else if (dynamic_cast<IIfcTrimmedCurve^>(segment->ParentCurve))
			{
				IIfcTrimmedCurve^ tc = dynamic_cast<IIfcTrimmedCurve^>(segment->ParentCurve);
				Xbim::Ifc4::MeasureResource::IfcParameterValue^ t1 = dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(tc->Trim1[0]);
				Xbim::Ifc4::MeasureResource::IfcParameterValue^ t2 = dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(tc->Trim2[0]);
				return (double)t2->Value - (double)t1->Value;
			}
			return 1;
		}

		void XbimSolid::Init(IIfcSweptDiskSolid^ repItem, ILogger^ logger)
		{

			//Build the directrix
			IModelFactors^ mf = repItem->Model->ModelFactors;

			XbimWire^ sweep = gcnew XbimWire(repItem->Directrix, logger);
			if (!sweep->IsValid) return;

			// comments on the interpretation of the params are found here:
			// https://sourceforge.net/p/ifcexporter/discussion/general/thread/7cc44b69/?limit=25
			// if the directix is:
			// - a line, just use the lenght
			// - a IFCCOMPOSITECURVE then the parameter 
			//   for each line add 0 to 1
			//   for each arc add the angle
			//

			if (dynamic_cast<IIfcLine^>(repItem->Directrix)) //params are different
			{
				IIfcLine ^  line = (IIfcLine^)(repItem->Directrix);
				double mag = line->Dir->Magnitude;
				if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value *mag, repItem->EndParam.Value * mag, mf->Precision, logger);
				else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value*mag, sweep->Length, mf->Precision, logger);
				else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(0, repItem->EndParam.Value * mag, mf->Precision, logger);
			}
			else if (dynamic_cast<IIfcCompositeCurve^>(repItem->Directrix)) //params are different
			{

				double startPar = 0;
				double endPar = double::PositiveInfinity;
				if (repItem->StartParam.HasValue)
					startPar = repItem->StartParam.Value;
				if (repItem->EndParam.HasValue)
					endPar = repItem->EndParam.Value;
				double occStart = 0;
				double occEnd = 0;
				double totCurveLen = 0;

				// for each segment we encounter, we will see if the threshold falls within its lenght
				//
				IIfcCompositeCurve ^  curve = (IIfcCompositeCurve^)(repItem->Directrix);
				for each (IIfcCompositeCurveSegment^ segment in curve->Segments)
				{
					XbimWire^ segWire = gcnew XbimWire(segment, logger);
					double wireLen = segWire->Length;       // this is the lenght to add to the OCC command if we use all of the segment
					double segValue = SegLenght(segment);   // this is the IFC size of the segment
					totCurveLen += wireLen;
					//System::Diagnostics::Debug::Write("wireLen:\t");
					//System::Diagnostics::Debug::Write(wireLen);
					//System::Diagnostics::Debug::Write("\tsegValue:\t");
					//System::Diagnostics::Debug::Write(segValue);
					//System::Diagnostics::Debug::Write("\r\n");

					if (startPar > 0)
					{
						double ratio = Math::Min(startPar / segValue, 1.0);
						startPar -= ratio * segValue; // reduce the outstanding amount (since it's been accounted for in the segment just processed)
						occStart += ratio * wireLen; // progress the occ amount by the ratio of the lenght
					}

					if (endPar > 0)
					{
						double ratio = Math::Min(endPar / segValue, 1.0);
						endPar -= ratio * segValue; // reduce the outstanding amount (since it's been accounted for in the segment just processed)
						occEnd += ratio * wireLen; // progress the occ amount by the ratio of the lenght
					}
				}					
				// only trim if needed either from start or end
				if (occStart > 0 || occEnd < totCurveLen)
					sweep = (XbimWire^)sweep->Trim(occStart, occEnd, mf->Precision, logger);
			}
			else 
			{
				if (repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, repItem->EndParam.Value, mf->Precision, logger);
				else if (repItem->StartParam.HasValue && !repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(repItem->StartParam.Value, sweep->Length, mf->Precision, logger);
				else if (!repItem->StartParam.HasValue && repItem->EndParam.HasValue)
					sweep = (XbimWire^)sweep->Trim(0, repItem->EndParam.Value, mf->Precision, logger);
			}

			IIfcSweptDiskSolidPolygonal^ polygonal = dynamic_cast<IIfcSweptDiskSolidPolygonal^>(repItem);
			if (polygonal != nullptr && polygonal->FilletRadius.HasValue)
			{
				if (!sweep->FilletAll((double)polygonal->FilletRadius.Value))
					XbimGeometryCreator::LogWarning(logger, repItem, "Sweep Could not be corectly filleted");
			}

			//make the outer wire
			XbimPoint3D s = sweep->Start;
			gp_Pnt startPnt(s.X, s.Y, s.Z);

			TopoDS_Edge edge;
			Standard_Real uoe;
			BRepAdaptor_CompCurve cc(sweep);
			cc.Edge(0, edge, uoe);
			Standard_Real lp, fp;
			Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, fp, lp);
			gp_Pnt p1;
			gp_Vec tangent;
			curve->D1(0, p1, tangent);

			gp_Ax2 axCircle(startPnt, tangent);
			gp_Circ outer(axCircle, repItem->Radius);
			Handle(Geom_Circle) hOuter = GC_MakeCircle(outer);
			TopoDS_Edge outerEdge = BRepBuilderAPI_MakeEdge(hOuter);

			TopoDS_Wire outerWire = BRepBuilderAPI_MakeWire(outerEdge);




			BRepOffsetAPI_MakePipeShell pipeMaker1(sweep);
			pipeMaker1.Add(outerWire, Standard_False, Standard_True);
			pipeMaker1.SetTransitionMode(BRepBuilderAPI_Transformed);
			try
			{
				pipeMaker1.Build();
			}
			catch (...) //if transformed mode fails try 
			{
				XbimGeometryCreator::LogError(logger, repItem, "Could not construct IfcSweptDiskSolid");
				return;
			}


			if (pipeMaker1.IsDone())
			{

				if (!pipeMaker1.MakeSolid()) //we cannot make a solid or it is already a solid
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Could not construct IfcSweptDiskSolidPolygonal");
					return;
				}

				TopoDS_Solid pipe = TopoDS::Solid(pipeMaker1.Shape());

				if (!sweep->IsClosed && repItem->InnerRadius.HasValue && repItem->InnerRadius.Value > 0 && repItem->InnerRadius.Value < repItem->Radius)
				{
					bool isClosed = pipeMaker1.FirstShape().ShapeType() == TopAbs_WIRE; //if the first is stil a wire the shape is closed, it should be a face to make a valid solid that is open

					//now add inner wire if it is defined
					gp_Circ inner(axCircle, repItem->InnerRadius.Value);
					Handle(Geom_Circle) hInner = GC_MakeCircle(inner);
					TopoDS_Edge innerEdge = BRepBuilderAPI_MakeEdge(hInner);
					BRepBuilderAPI_MakeWire innerWire;
					innerWire.Add(innerEdge);
					BRepOffsetAPI_MakePipeShell pipeMaker2(sweep);
					pipeMaker2.SetTransitionMode(BRepBuilderAPI_RightCorner);
					pipeMaker2.Add(innerWire.Wire(), Standard_False, Standard_True);
					pipeMaker2.Build();
					if (pipeMaker2.IsDone() && !isClosed) //no pint in adding a void to a pipe solid that is closed
					{
						if (!pipeMaker2.MakeSolid()) //we cannot make a solid
						{
							XbimGeometryCreator::LogWarning(logger, repItem, "Could not construct inner sweep of IfcSweptDiskSolidPolygonal");
						}
						else
						{
							TopoDS_Solid pipeInner = TopoDS::Solid(pipeMaker2.Shape());
							BRep_Builder bs;
							TopoDS_Shell shell;
							bs.MakeShell(shell);
							TopTools_IndexedMapOfShape pipeMap;
							TopExp::MapShapes(pipe, TopAbs_FACE, pipeMap);
							TopoDS_Face firstFace;
							TopoDS_Face lastFace;
							for (int i = 1; i <= pipeMap.Extent(); i++)
							{
								const TopoDS_Face & f = TopoDS::Face(pipeMap(i));
								if (f.IsSame(pipeMaker1.FirstShape()))
									firstFace = f;
								else if (f.IsSame(pipeMaker1.LastShape()))
									lastFace = f;
								else
									bs.Add(shell, f);
							}
							TopTools_IndexedMapOfShape innerPipeMap;
							TopExp::MapShapes(pipeInner, TopAbs_FACE, innerPipeMap);
							for (int i = 1; i <= innerPipeMap.Extent(); i++)
							{
								const TopoDS_Face & f = TopoDS::Face(innerPipeMap(i));

								if (f.IsEqual(pipeMaker2.FirstShape()))
								{
									BRepBuilderAPI_MakeFace firstMaker(firstFace);
									TopoDS_Wire w = BRepTools::OuterWire(f);
									w.Reverse();
									firstMaker.Add(w);
									firstFace = firstMaker.Face();
								}
								else if (f.IsEqual(pipeMaker2.LastShape()))
								{
									BRepBuilderAPI_MakeFace lastMaker(lastFace);
									TopoDS_Wire w = BRepTools::OuterWire(f);
									w.Reverse();
									lastMaker.Add(w);
									lastFace = lastMaker.Face();
								}
								else
									bs.Add(shell, f.Reversed());

							}
							TopoDS_Solid solid;
							bs.MakeSolid(solid);

							bs.Add(shell, firstFace);
							bs.Add(shell, lastFace);
							bs.Add(solid, shell);

							BRepClass3d_SolidClassifier SC(solid);
							SC.PerformInfinitePoint(Precision::Confusion());
							if (SC.State() == TopAbs_IN) {
								bs.MakeSolid(solid);
								shell.Reverse();
								bs.Add(solid, shell);
							}
							solid.Closed(Standard_True);
							pipe = solid;

						}
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, repItem, "Inner loop could not be constructed");
					}
				}
				pSolid = new TopoDS_Solid();
				*pSolid = pipe;
				pSolid->Closed(Standard_True);
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, repItem->Model->ModelFactors->Precision);
				return;

			}
			else
			{
				XbimGeometryCreator::LogWarning(logger, repItem, "Could not be constructed");
			}
		}



		void XbimSolid::Init(IIfcBoundingBox^ box, ILogger^ /*logger*/)
		{
			double precision = box->Model->ModelFactors->Precision;
			double x = Math::Max(box->XDim, precision);
			double y = Math::Max(box->YDim, precision);
			double z = Math::Max(box->ZDim, precision);
			gp_Ax2 	gpax2(gp_Pnt(box->Corner->X, box->Corner->Y, box->Corner->Z), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
			BRepPrimAPI_MakeBox boxMaker(gpax2, x, y, z);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(boxMaker.Shape());
			ShapeFix_ShapeTolerance FTol;
			FTol.LimitTolerance(*pSolid, box->Model->ModelFactors->Precision);
		}



		void XbimSolid::Init(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^ logger)
		{
			IIfcSphere^ sphere = dynamic_cast<IIfcSphere^>(IIfcSolid);
			if (sphere != nullptr) return Init(sphere, logger);
			IIfcBlock^ block = dynamic_cast<IIfcBlock^>(IIfcSolid);
			if (block != nullptr) return Init(block, logger);
			IIfcRightCircularCylinder^ cylinder = dynamic_cast<IIfcRightCircularCylinder^>(IIfcSolid);
			if (cylinder != nullptr) return Init(cylinder, logger);
			IIfcRightCircularCone^ cone = dynamic_cast<IIfcRightCircularCone^>(IIfcSolid);
			if (cone != nullptr) return Init(cone, logger);
			IIfcRectangularPyramid^ pyramid = dynamic_cast<IIfcRectangularPyramid^>(IIfcSolid);
			if (pyramid != nullptr) return Init(pyramid, logger);
			throw gcnew NotImplementedException(String::Format("IIfcCsgPrimitive3D of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));
		}



		void XbimSolid::Init(IIfcSphere^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeSphere sphereMaker(gpax3.Ax2(), IIfcSolid->Radius);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(sphereMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcBlock^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeBox boxMaker(gpax3.Ax2(), IIfcSolid->XLength, IIfcSolid->YLength, IIfcSolid->ZLength);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(boxMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcRightCircularCylinder^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeCylinder cylinderMaker(gpax3.Ax2(), IIfcSolid->Radius, IIfcSolid->Height);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(cylinderMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcRightCircularCone^ IIfcSolid, ILogger^ /*logger*/)
		{
			gp_Ax3 	gpax3 = XbimConvert::ToAx3(IIfcSolid->Position);
			BRepPrimAPI_MakeCone coneMaker(gpax3.Ax2(), IIfcSolid->BottomRadius, 0., IIfcSolid->Height);
			pSolid = new TopoDS_Solid();
			*pSolid = TopoDS::Solid(coneMaker.Shape());
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcRectangularPyramid^ IIfcSolid, ILogger^ /*logger*/)
		{

			double xOff = IIfcSolid->XLength / 2;
			double yOff = IIfcSolid->YLength / 2;
			double precision = IIfcSolid->Model->ModelFactors->Precision;

			gp_Pnt bl(0, 0, 0);
			gp_Pnt br(IIfcSolid->XLength, 0, 0);
			gp_Pnt tr(IIfcSolid->XLength, IIfcSolid->YLength, 0);
			gp_Pnt tl(0, IIfcSolid->YLength, 0);
			gp_Pnt p(xOff, yOff, IIfcSolid->Height);
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
			BRepBuilderAPI_MakeFace afaceBlder(gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, -1)), baseWire, Standard_True);
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
			Move(IIfcSolid->Position);
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
		}

		void XbimSolid::Init(IIfcTriangulatedFaceSet ^ IIfcSolid, ILogger ^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(IIfcSolid, logger);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew();
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, IIfcSolid->Model->ModelFactors->Precision);
			}
		}

		void XbimSolid::Init(IIfcFaceBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew();
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, solid->Model->ModelFactors->Precision);
			}
		}

		void XbimSolid::Init(IIfcShellBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			if (comp->IsValid)
			{
				if (comp->Solids->Count == 1) //we have one solid just return it and ignore extraneous faces
				{
					pSolid = new TopoDS_Solid();
					*pSolid = (XbimSolid^)comp->Solids->First;
					return;
				}
				comp->Sew();
				XbimShell^ shell = (XbimShell^)comp->MakeShell();
				pSolid = new TopoDS_Solid();
				*pSolid = (XbimSolid^)(shell->CreateSolid());
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(*pSolid, solid->Model->ModelFactors->Precision);
			}
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

		bool XbimSolid::IsEmpty::get()
		{
			if (!IsValid) return true;
			TopTools_IndexedMapOfShape shellMap;
			TopExp::MapShapes(*pSolid, TopAbs_SHELL, shellMap);
			if (shellMap.Extent() == 0) return true;
			for (int i = 1; i <= shellMap.Extent(); i++)
			{
				TopTools_IndexedMapOfShape faceMap;
				TopExp::MapShapes(TopoDS::Shell(shellMap(i)), TopAbs_FACE, faceMap);
				if (faceMap.Extent() > 0) return false; //if we find a face in a shell we have something to work with
			}
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
			for (TopExp_Explorer exp(*pSolid, TopAbs_FACE); exp.More(); exp.Next())
			{
				Handle(Geom_Surface) s = BRep_Tool::Surface(TopoDS::Face(exp.Current()));
				GeomLib_IsPlanarSurface tester(s);
				if (!tester.IsPlanar())
					return false;
			}
			GC::KeepAlive(this);
			//all faces are planar
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
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_True);
			GC::KeepAlive(this);
			return gcnew XbimSolid(TopoDS::Solid(gTran.Shape()));
		}

		IXbimGeometryObject^ XbimSolid::TransformShallow(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_False);
			GC::KeepAlive(this);
			return gcnew XbimSolid(TopoDS::Solid(gTran.Shape()));
		}

		IXbimSolidSet^ XbimSolid::Cut(IXbimSolidSet^ toCut, double tolerance, ILogger^ logger)
		{
			if (toCut->Count == 0) return gcnew XbimSolidSet(this);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Cut(toCut, tolerance, logger);
		}


		IXbimSolidSet^ XbimSolid::Cut(IXbimSolid^ toCut, double tolerance, ILogger^logger)
		{
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Cut(gcnew XbimSolidSet(toCut), tolerance, logger);
		}

		IXbimSolidSet^ XbimSolid::Intersection(IXbimSolidSet^ toIntersect, double tolerance, ILogger^ logger)
		{
			if (toIntersect->Count == 0) return gcnew XbimSolidSet(this);
			if (toIntersect->Count == 1) return this->Intersection(toIntersect->First, tolerance, logger);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Intersection(toIntersect, tolerance, logger);
		}

		IXbimSolidSet^ XbimSolid::Intersection(IXbimSolid^ toIntersect, double /*tolerance*/, ILogger^ logger)
		{
			if (!IsValid || !toIntersect->IsValid) return XbimSolidSet::Empty;
			XbimSolid^ solidIntersect = dynamic_cast<XbimSolid^>(toIntersect);
			if (solidIntersect == nullptr)
			{
				XbimGeometryCreator::LogWarning(logger, toIntersect, "Invalid operation. Only solid shapes can be intersected with another solid");
				return gcnew XbimSolidSet(this); // the result would be no change so return this			
			}
			/*ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solidIntersect, tolerance);
			fixTol.SetTolerance(this, tolerance);*/
			String^ err = "";
			try
			{
				BRepAlgoAPI_Common boolOp(this, solidIntersect);
				if (boolOp.HasErrors() == Standard_False)
					return gcnew XbimSolidSet(boolOp.Shape());
			}
			catch (const std::exception &exc)
			{
				err = gcnew String(exc.what());
			}
			XbimGeometryCreator::LogWarning(logger, toIntersect, "Intersect operation failed,{0}", err);
			return XbimSolidSet::Empty;
		}

		IXbimSolidSet^ XbimSolid::Union(IXbimSolidSet^ toUnion, double tolerance, ILogger^ logger)
		{
			if (toUnion->Count == 0) return gcnew XbimSolidSet(this);
			if (toUnion->Count == 1) return this->Union(toUnion->First, tolerance, logger);
			XbimSolidSet^ thisSolidSet = gcnew XbimSolidSet(this);
			return thisSolidSet->Union(toUnion, tolerance, logger);
		}

		IXbimSolidSet^ XbimSolid::Union(IXbimSolid^ toUnion, double /*tolerance*/, ILogger^ logger)
		{
			if (!IsValid || !toUnion->IsValid) return XbimSolidSet::Empty;
			XbimSolid^ solidUnion = dynamic_cast<XbimSolid^>(toUnion);
			if (solidUnion == nullptr)
			{


				XbimGeometryCreator::LogWarning(logger, toUnion, "Invalid operation. Only solid shapes can be unioned with another solid");
				return gcnew XbimSolidSet(this); // the result would be no change so return this
			}


			/*ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solidUnion, tolerance);
			fixTol.SetTolerance(this, tolerance);*/
			String^ err = "";
			try
			{
				BRepAlgoAPI_Fuse boolOp(this, solidUnion);
				if (boolOp.HasErrors() == Standard_False)
					return gcnew XbimSolidSet(boolOp.Shape());
			}
			catch (const std::exception &exc)
			{
				err = gcnew String(exc.what());
			}
			XbimGeometryCreator::LogWarning(logger, toUnion, "Boolean Union operation failed, {0}", err);
			return XbimSolidSet::Empty;
		}


		IXbimFaceSet^ XbimSolid::Section(IXbimFace^ toSection, double tolerance, ILogger^ logger)
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
				for (TopExp_Explorer exp2(closed, TopAbs_WIRE); exp2.More(); exp2.Next())
				{
					ShapeFix_Wire wireFixer(TopoDS::Wire(exp2.Current()), faceSection, tolerance);
					wireFixer.Perform();
					TopoDS_Wire w = wireFixer.Wire();
					fr.Add(w);
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
			XbimGeometryCreator::LogWarning(logger, toSection, "Boolean Section operation has failed to create a section");

			return XbimFaceSet::Empty;
		}

		void XbimSolid::SaveAsBrep(String^ fileName)
		{
			if (IsValid)
			{
				XbimOccWriter^ occWriter = gcnew XbimOccWriter();
				occWriter->Write(this, fileName);
			}
		}

#pragma endregion

#pragma region Boolean operations

		void XbimSolid::Move(TopLoc_Location loc)
		{
			if (IsValid) pSolid->Move(loc);
		}

		void XbimSolid::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimConvert::ToTransform(position);
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

		void XbimSolid::CorrectOrientation()
		{
			if (IsValid)
			{
				BRepClass3d_SolidClassifier class3d(*pSolid);
				class3d.PerformInfinitePoint(Precision::Confusion());
				if (class3d.State() == TopAbs_IN) this->Reverse();
			}
		}



		bool XbimSolid::FixTopology(double tolerance)
		{
			if (IsValid)
			{

				if (BRepCheck_Analyzer(*pSolid, Standard_True).IsValid() == Standard_False)
				{
					ShapeFix_Shape shapeFixer(*pSolid);
					shapeFixer.SetPrecision(tolerance * 10);
					shapeFixer.LimitTolerance(tolerance * 10);
					shapeFixer.FixSolidMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixIntersectingWiresMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixOrientationMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixWireTool()->FixAddCurve3dMode() = Standard_True;
					shapeFixer.FixFaceTool()->FixWireTool()->FixIntersectingEdgesMode() = Standard_True;
					Handle(ShapeFix_Solid) solidTool = shapeFixer.FixSolidTool();
					if (shapeFixer.Perform())
					{
						//	if (BRepCheck_Analyzer(shapeFixer.Shape(), Standard_True).IsValid() == Standard_True)
						{
							if (solidTool->Solid().ShapeType() == TopAbs_SOLID)
							{
								*pSolid = TopoDS::Solid(solidTool->Solid());
								//ShapeUpgrade_UnifySameDomain unifier(ts);
								//unifier.SetAngularTolerance(0.00174533); //1 tenth of a degree
								//unifier.SetLinearTolerance(tolerance);
								//try
								//{
								//	//sometimes unifier crashes
								//	unifier.Build();
								//	if (unifier.Shape().ShapeType() == TopAbs_SOLID)
								//	{
								//		*pSolid = (TopoDS::Solid(unifier.Shape()));
								//		return true; //only place for total success, all else is a failure
								//	}
								//}
								//catch (...) //just catch its a fail
								//{
								//}
							}
						}
					}

				}
				else
					return true;
			}
			return false;
		}

		XbimGeometryObject ^ XbimSolid::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(*pSolid, trans, Standard_True); //make a copy of underlying shape
				GC::KeepAlive(this);
				return gcnew XbimSolid(TopoDS::Solid(tr.Shape()), Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(*pSolid, trans, Standard_False); //do not make a copy of underlying shape
				GC::KeepAlive(this);
				return gcnew XbimSolid(TopoDS::Solid(tr.Shape()), Tag);
			}
		}

		XbimGeometryObject ^ XbimSolid::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimSolid^ copy = gcnew XbimSolid(*pSolid, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject ^ XbimSolid::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimSolid^ copy = gcnew XbimSolid(*pSolid, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			copy->Move(loc);
			return copy;
		}


#pragma endregion
	}


}