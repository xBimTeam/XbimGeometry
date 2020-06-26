#include "XbimCompound.h"
#include "XbimGeometryCreator.h"
#include "XbimGeometryObjectSet.h"
#include "XbimOccWriter.h"
#include "XbimSolidSet.h"
#include "XbimShellSet.h"
#include "XbimFaceSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimConvert.h"
#include "XbimGeometryObjectSet.h"
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_FastSewing.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepPrim_Builder.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepAlgoAPI_Check.hxx>
#include <ShapeFix_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <ShapeFix_Shell.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_FixSmallSolid.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BOPAlgo_MakerVolume.hxx>
#include <ShapeAnalysis_Shell.hxx>
#include <ShapeUpgrade_ShellSewing.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepBuilderAPI_CellFilter.hxx>
#include <BRepBuilderAPI_VertexInspector.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <ShapeUpgrade_RemoveInternalWires.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepMesh_VertexInspector.hxx>
#include <Geom_BSplineCurve.hxx>
#include <ShapeAnalysis.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GC_MakeSegment.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeInteger.hxx>
#include <TopTools_DataMapOfShapeListOfInteger.hxx>
#include <GeomLib_Tool.hxx>
#include <BRepGProp_Face.hxx>
#include <TColStd_DataMapOfIntegerListOfInteger.hxx>
#include <ShapeAnalysis_WireOrder.hxx>
#include <BRepCheck_Shell.hxx>
// #include <ShapeBuild_ReShape.hxx> // this was suggeste in PR79 - but it does not seem to make the difference with OCC72

using namespace System;
using namespace System::Linq;
using namespace Xbim::Common;
using namespace Xbim::Common::XbimExtensions;
using namespace Xbim::Ifc4::Interfaces;
using namespace System::Diagnostics;

namespace Xbim
{
	namespace Geometry
	{
		XbimCompound::XbimCompound(double sewingTolerance)
		{
			_sewingTolerance = sewingTolerance;
		}

		/*Ensures native pointers are deleted and garbage collected*/
		void XbimCompound::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Compound*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

		IEnumerator<IXbimGeometryObject^>^ XbimCompound::GetEnumerator()
		{
			//add all top level objects in to the collection, ignore nested objects
			List<IXbimGeometryObject^>^ result = gcnew List<IXbimGeometryObject^>(1);
			if (!IsValid) return result->GetEnumerator();

			for (TopExp_Explorer expl(*pCompound, TopAbs_SOLID); expl.More(); expl.Next())
			{
				XbimSolid^ solid = gcnew XbimSolid(TopoDS::Solid(expl.Current()));
				//solid->Move(loc);
				solid->Tag = Tag;
				result->Add(solid);
			}

			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL, TopAbs_SOLID); expl.More(); expl.Next())
			{
				XbimShell^ shell = gcnew XbimShell(TopoDS::Shell(expl.Current()));
				//shell->Move(loc);
				shell->Tag = Tag;
				result->Add(shell);
			}

			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE, TopAbs_SHELL); expl.More(); expl.Next())
			{
				XbimFace^ face = gcnew XbimFace(TopoDS::Face(expl.Current()));
				//face->Move(loc);
				face->Tag = Tag;
				result->Add(face);
			}

			return result->GetEnumerator();
		}

		int XbimCompound::Count::get()
		{
			if (pCompound == nullptr) return 0;
			int count = 0;
			for (TopExp_Explorer expl(*pCompound, TopAbs_SOLID); expl.More(); expl.Next())
				count++;
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL, TopAbs_SOLID); expl.More(); expl.Next())
				count++;
			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE, TopAbs_SHELL); expl.More(); expl.Next())
				count++;
			GC::KeepAlive(this);
			return count;
		}

		IXbimGeometryObject^ XbimCompound::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimConvert::ToTransform(matrix3D));
			TopoDS_Compound temp = TopoDS::Compound(gTran.Shape());
			GC::KeepAlive(this);
			return gcnew XbimCompound(temp, IsSewn, _sewingTolerance);
		}

		IXbimGeometryObject^ XbimCompound::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Compound shallowCopy = TopoDS::Compound(pCompound->Moved(XbimConvert::ToTransform(matrix3D)));
			GC::KeepAlive(this);
			return gcnew XbimCompound(shallowCopy, IsSewn, _sewingTolerance);
		}

		XbimRect3D XbimCompound::BoundingBox::get()
		{
			if (pCompound == nullptr || pCompound->IsNull())
				return XbimRect3D::Empty;

			const TopoDS_Compound& occComp = *pCompound;
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			bool isVoid = false;
			try
			{
				Bnd_Box pBox;
				BRepBndLib::Add(occComp, pBox);
				isVoid = pBox.IsVoid();
				if (!isVoid)
					pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			}
			catch (Standard_Failure sf)
			{
				//String^ err = gcnew String(sf.GetMessageString());
				return XbimRect3D::Empty;
			}
			if (isVoid)
				return XbimRect3D::Empty;
			else
				return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		IXbimGeometryObject^ XbimCompound::First::get()
		{
			if (!IsValid) return nullptr;
			for (TopExp_Explorer expl(*pCompound, TopAbs_SOLID); expl.More();)
				return gcnew XbimSolid(TopoDS::Solid(expl.Current()));
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL, TopAbs_SOLID); expl.More();)
				return gcnew XbimShell(TopoDS::Shell(expl.Current()));
			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE, TopAbs_SHELL); expl.More();)
				return gcnew XbimFace(TopoDS::Face(expl.Current()));
			GC::KeepAlive(this);
			return nullptr;
		}


		XbimCompound::XbimCompound(IIfcConnectedFaceSet^ faceSet, ILogger^ logger)
		{
			_sewingTolerance = faceSet->Model->ModelFactors->Precision;
			Init(faceSet, logger);
		}

		XbimCompound::XbimCompound(IIfcShellBasedSurfaceModel^ sbsm, ILogger^ logger)
		{
			_sewingTolerance = sbsm->Model->ModelFactors->Precision;
			Init(sbsm, logger);
		}

		XbimCompound::XbimCompound(IIfcFaceBasedSurfaceModel^ fbsm, ILogger^ logger)
		{
			_sewingTolerance = fbsm->Model->ModelFactors->Precision;
			Init(fbsm, logger);
		}

		XbimCompound::XbimCompound(IIfcManifoldSolidBrep^ solid, ILogger^ logger)
		{
			_sewingTolerance = solid->Model->ModelFactors->Precision;
			Init(solid, logger);
		}
		XbimCompound::XbimCompound(IIfcFacetedBrep^ solid, ILogger^ logger)
		{
			_sewingTolerance = solid->Model->ModelFactors->Precision;
			Init(solid, logger);
		}

		XbimCompound::XbimCompound(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger)
		{
			_sewingTolerance = solid->Model->ModelFactors->Precision;
			Init(solid, logger);
		}
		XbimCompound::XbimCompound(IIfcAdvancedBrep^ solid, ILogger^ logger)
		{
			_sewingTolerance = solid->Model->ModelFactors->Precision;
			Init(solid, logger);
		}

		XbimCompound::XbimCompound(IIfcAdvancedBrepWithVoids^ solid, ILogger^ logger)
		{
			_sewingTolerance = solid->Model->ModelFactors->Precision;
			Init(solid, logger);
		}

		XbimCompound::XbimCompound(IIfcClosedShell^ solid, ILogger^ logger)
		{
			_sewingTolerance = solid->Model->ModelFactors->Precision;
			Init(solid, logger);
		}

		XbimCompound::XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance)
		{
			pCompound = new TopoDS_Compound();
			*pCompound = compound;
			_isSewn = sewn;
			_sewingTolerance = tolerance;

		}
		XbimCompound::XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance, Object^ tag) :XbimCompound(compound, sewn, tolerance)
		{
			Tag = tag;
		}

		void XbimCompound::Move(TopLoc_Location loc)
		{
			if (IsValid) pCompound->Move(loc);
		}


		void XbimCompound::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimConvert::ToTransform(position);
			pCompound->Move(toPos);
		}

		XbimGeometryObject^ XbimCompound::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				GC::KeepAlive(this);
				return gcnew XbimCompound(TopoDS::Compound(tr.Shape()), _isSewn, _sewingTolerance);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				GC::KeepAlive(this);
				return gcnew XbimCompound(TopoDS::Compound(tr.Shape()), _isSewn, _sewingTolerance);
			}
		}

		XbimGeometryObject^ XbimCompound::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimCompound^ copy = gcnew XbimCompound(this, _isSewn, _sewingTolerance, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject^ XbimCompound::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimCompound^ copy = gcnew XbimCompound(this, _isSewn, _sewingTolerance, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			copy->Move(loc);
			return copy;
		}

		XbimCompound::XbimCompound(IIfcTriangulatedFaceSet^ faceSet, ILogger^ logger)
		{
			_sewingTolerance = faceSet->Model->ModelFactors->Precision;
			Init(faceSet, logger);
		}

		XbimCompound::XbimCompound(IIfcPolygonalFaceSet^ faceSet, ILogger^ logger)
		{
			_sewingTolerance = faceSet->Model->ModelFactors->Precision;
			IList<IIfcFace^>^ faceList = gcnew XbimPolygonalFaceSet(faceSet);
			//if the face set has more than max faces just abandon and try and mesh
			if (faceList->Count > MaxFacesToSew)
			{
				XbimGeometryFaceSetTooLargeException^ except = gcnew XbimGeometryFaceSetTooLargeException();
				except->Data->Add("LargeFaceSetLabel", faceSet->EntityLabel);
				except->Data->Add("LargeFaceSetType", faceSet->GetType()->Name);
				throw except;
			}
			Init(faceList, faceSet, logger);
		}

#pragma region Initialisers

		void XbimCompound::Init(IIfcFaceBasedSurfaceModel^ fbsm, ILogger^ logger)
		{
			pCompound = new TopoDS_Compound();
			BRep_Builder builder;
			builder.MakeCompound(*pCompound);
			for each (IIfcConnectedFaceSet ^ faceSet in fbsm->FbsmFaces)
			{
				XbimCompound^ compound = gcnew XbimCompound(faceSet, logger);
				for each (IXbimGeometryObject ^ geom in compound)
				{
					if (dynamic_cast<XbimSolid^>(geom))
						builder.Add(*pCompound, (XbimSolid^)geom);
					else if (dynamic_cast<XbimShell^>(geom))
					{
						XbimShell^ shell = (XbimShell^)geom;
						if (shell->IsClosed)
						{
							XbimSolid^ solid = (XbimSolid^)shell->MakeSolid();
							if (solid->IsValid)
							{
								builder.Add(*pCompound, solid);
								continue;
							}
						}
						builder.Add(*pCompound, shell);
					}
					else if (dynamic_cast<XbimFace^>(geom))
						builder.Add(*pCompound, (XbimFace^)geom);
				}
			}
		}

		void XbimCompound::Init(IIfcShellBasedSurfaceModel^ sbsm, ILogger^ logger)
		{
			List<XbimShell^>^ shells = gcnew List<XbimShell^>();
			for each (IIfcShell ^ shell in sbsm->SbsmBoundary)
			{
				//List<IIfcFace^>^ faces = gcnew List<IIfcFace^>();
				//get the faces
				IIfcConnectedFaceSet^ faceSet = dynamic_cast<IIfcConnectedFaceSet^>(shell);
				if (faceSet != nullptr) //this should never fail
				{
					XbimCompound^ occShell;
					if (dynamic_cast<IIfcClosedShell^>(shell))
						occShell = gcnew XbimCompound((IIfcClosedShell^)shell, logger);
					else
						occShell = gcnew XbimCompound((IIfcOpenShell^)shell, logger);
					for each (XbimShell ^ s in occShell->Shells)
					{
						XbimShell^ nestedShell = (XbimShell^)s;
						if (nestedShell->IsValid && !nestedShell->IsEmpty) shells->Add(nestedShell);
					}
				}
			}
			if (shells->Count > 0)
			{
				BRep_Builder b;
				pCompound = new TopoDS_Compound();
				b.MakeCompound(*pCompound);
				for each (XbimShell ^ s in shells)
				{
					b.Add(*pCompound, s);
				}
			}
		}

		void XbimCompound::Init(IIfcConnectedFaceSet^ faceSet, ILogger^ logger)
		{
			if (!Enumerable::Any(faceSet->CfsFaces))
			{
				XbimGeometryCreator::LogWarning(logger, faceSet, "Empty face set");
				return;
			}
			//if the face set has more than max faces just abandon and try and mesh
			if (faceSet->CfsFaces->Count > MaxFacesToSew)
			{
				XbimGeometryFaceSetTooLargeException^ except = gcnew XbimGeometryFaceSetTooLargeException();
				except->Data->Add("LargeFaceSetLabel", faceSet->EntityLabel);
				except->Data->Add("LargeFaceSetType", faceSet->GetType()->Name);
				throw except;
			}
			Init(faceSet->CfsFaces, faceSet, logger);

		}


		void XbimCompound::Init(IIfcManifoldSolidBrep^ solid, ILogger^ logger)
		{
			IIfcFacetedBrep^ facetedBrep = dynamic_cast<IIfcFacetedBrep^>(solid);
			if (facetedBrep != nullptr) return Init(facetedBrep, logger);

			IIfcAdvancedBrep^ advancedBrep = dynamic_cast<IIfcAdvancedBrep^>(solid);
			if (advancedBrep != nullptr) return Init(advancedBrep, logger);

			throw gcnew NotImplementedException("Sub-Type of IIfcManifoldSolidBrep is not implemented");
		}

		//Many of the Brep defintions are not good, often they have faces missing and holes missing, so we cannot guarantee to build a valid OCC solid
		//we make the best attempt to be a shell that is as near as possible to the solid
		void XbimCompound::Init(IIfcAdvancedBrep^ solid, ILogger^ logger)
		{
			IIfcAdvancedBrepWithVoids^ advancedBrepWithVoids = dynamic_cast<IIfcAdvancedBrepWithVoids^>(solid);
			if (advancedBrepWithVoids != nullptr) return Init(advancedBrepWithVoids, logger);
			BRep_Builder b;
			TopoDS_Shell occOuterShell = InitAdvancedFaces(solid->Outer->CfsFaces, logger);
			if (occOuterShell.IsNull()) return;

			pCompound = new TopoDS_Compound();
			b.MakeCompound(*pCompound);
			if (occOuterShell.Closed())
			{
				BRepBuilderAPI_MakeSolid solidmaker;
				solidmaker.Add(occOuterShell);
				solidmaker.Build();
				if (solidmaker.IsDone())
				{
					TopoDS_Solid solid = solidmaker.Solid();
					solid.Closed(true);
					solid.Checked(true);
					b.Add(*pCompound, solid);
				}
				pCompound->Closed(true);
				pCompound->Checked(true);
				return;
			}

			//advanced breps are always solids, so to make sure we have highest form, sometime we get multiple solids
			try
			{
				ShapeFix_Shell shellFixer(occOuterShell);
				shellFixer.Perform();

				TopTools_IndexedMapOfShape shellMap;
				TopExp::MapShapes(shellFixer.Shape(), TopAbs_SHELL, shellMap);
				for (int ishell = 1; ishell <= shellMap.Extent(); ++ishell)
				{
					// Build solid
					BRepBuilderAPI_MakeSolid solidmaker;
					const TopoDS_Shell& shell = TopoDS::Shell(shellMap(ishell));
					solidmaker.Add(shell);
					solidmaker.Build();
					if (solidmaker.IsDone())
					{
						b.Add(*pCompound, solidmaker.Solid());
					}
				}
			}
			catch (Standard_Failure sf)
			{
				String^ err = gcnew String(sf.GetMessageString());
				XbimGeometryCreator::LogWarning(logger, solid, "Failed to create  IfcAdvancedBrep: " + err);
				b.Add(*pCompound, occOuterShell); //just add what we have
			}
		}

		void XbimCompound::Init(IIfcFacetedBrep^ solid, ILogger^ logger)
		{
			if (solid->Outer->CfsFaces->Count < 4) // if we have 3 or less planar faces it cannot form a valid solid
			{
				XbimGeometryCreator::LogWarning(logger, solid, "IfcFacetedBrep has less than 4 planar faces it cannot be a correct closed shell");
				return;
			}

			IIfcFacetedBrepWithVoids^ facetedBrepWithVoids = dynamic_cast<IIfcFacetedBrepWithVoids^>(solid);
			if (facetedBrepWithVoids != nullptr) return Init(facetedBrepWithVoids, logger);
			Init(solid->Outer, logger);
		}

		void XbimCompound::Init(IIfcAdvancedBrepWithVoids^ brepWithVoids, ILogger^ logger)
		{
			BRep_Builder b;
			TopoDS_Shell outerShell = InitAdvancedFaces(brepWithVoids->Outer->CfsFaces, logger);
			XbimSolid^ theSolid;
			if (outerShell.Closed()) //if it is a closed shell make a solid
			{
				BRepBuilderAPI_MakeSolid solidmaker;
				solidmaker.Add(outerShell);
				theSolid = gcnew XbimSolid(solidmaker.Solid());
			}
			else
				XbimGeometryCreator::LogWarning(logger, brepWithVoids, "Cannot cut voids properly as outer shell is not a solid #{0} is not a solid.", brepWithVoids->EntityLabel);

			BRepBuilderAPI_MakeSolid builder(theSolid);
			for each (IIfcClosedShell ^ IIfcVoidShell in brepWithVoids->Voids)
			{
				XbimCompound^ voidShapes = gcnew XbimCompound(IIfcVoidShell, logger);
				XbimShell^ voidShell = (XbimShell^)voidShapes->MakeShell();
				if (!voidShell->IsClosed) //we have a shell that is not able to be made in to a solid
					XbimGeometryCreator::LogWarning(logger, brepWithVoids, "Cannot cut voids properly as the void #{0} is not a solid.", IIfcVoidShell->EntityLabel);
				builder.Add(voidShell);
			}
			if (builder.IsDone())
			{
				pCompound = new TopoDS_Compound();
				b.MakeCompound(*pCompound);
				b.Add(*pCompound, builder.Solid());
			}//leave the outer shell without the voids
			else
				XbimGeometryCreator::LogWarning(logger, brepWithVoids, "A correct shape could not be built and it has been ignored");
		}

		void XbimCompound::Init(IIfcFacetedBrepWithVoids^ brepWithVoids, ILogger^ logger)
		{
			XbimCompound^ shapes = gcnew XbimCompound(brepWithVoids->Outer, logger);
			XbimShell^ outerShell = (XbimShell^)shapes->MakeShell();
			if (!outerShell->IsClosed) //we have a shell that is not able to be made in to a solid
				XbimGeometryCreator::LogWarning(logger, brepWithVoids, "Can cut voids properly as the bounding shell #{0} is not a solid.", brepWithVoids->Outer->EntityLabel);
			BRepBuilderAPI_MakeSolid builder(outerShell);
			for each (IIfcClosedShell ^ IIfcVoidShell in brepWithVoids->Voids)
			{
				XbimCompound^ voidShapes = gcnew XbimCompound(IIfcVoidShell, logger);
				XbimShell^ voidShell = (XbimShell^)voidShapes->MakeShell();
				if (!voidShell->IsClosed) //we have a shell that is not able to be made in to a solid
					XbimGeometryCreator::LogWarning(logger, brepWithVoids, "Can cut voids properly as the void #{0} is not a solid.", IIfcVoidShell->EntityLabel);
				builder.Add(voidShell);
			}
			if (builder.IsDone())
			{
				pCompound = new TopoDS_Compound();
				BRep_Builder b;
				b.MakeCompound(*pCompound);
				b.Add(*pCompound, builder.Solid());
			}
			else
				XbimGeometryCreator::LogWarning(logger, brepWithVoids, "A correct shape could not be built and it has been ignored");
		}

		void XbimCompound::Init(IIfcClosedShell^ closedShell, ILogger^ logger)
		{

			BRep_Builder b;
			Init((IIfcConnectedFaceSet^)closedShell, logger);
			if (IsValid) //make it a closed solid if we can
			{
				bool hasMultiShellSolid = false;
				BRepBuilderAPI_MakeSolid solidmaker;
				//we have some shells that are a bunch of solids and we have some that are a single solid but a bunch of shells


				TopTools_IndexedMapOfShape solidMap;

				TopExp::MapShapes(*pCompound, TopAbs_SOLID, solidMap);

				TopExp_Explorer expShell(*pCompound, TopAbs_SHELL, TopAbs_SOLID);
				for (; expShell.More(); expShell.Next())
				{
					TopoDS_Shell shell = TopoDS::Shell(expShell.Current());

					ShapeAnalysis_Shell sa;
					sa.LoadShells(shell);
					if (sa.HasBadEdges())
					{
						ShapeFix_Shell sf(shell);
						if (sf.Perform())
						{
							shell = sf.Shell();
						}
					}
					if (!sa.HasFreeEdges()) //it is most likely a solid
					{
						BRepBuilderAPI_MakeSolid sm;
						sm.Add(shell); //adds and creates a solid
						if (sm.IsDone())
						{
							bool failed = false;
							TopoDS_Solid s = sm.Solid();
							try
							{
								BRepClass3d_SolidClassifier class3d(s);
								class3d.PerformInfinitePoint(Precision::Confusion());
								if (class3d.State() == TopAbs_IN) s.Reverse();
								solidMap.Add(s);
							}
							catch (...)
							{
								failed = true;
							}
							if (failed)
							{

								BRepTools::Write(s, "c:/tmp/1");
								BRepTools::Write(shell, "c:/tmp/2");
							}

							continue;
						}
					}
					solidmaker.Add(shell); //stick it into the one which will make a solid from a bunch of shells
					hasMultiShellSolid = true;

				}
				if (hasMultiShellSolid && solidmaker.IsDone())
				{
					//this is tricky we may have shells that dont make a solid, or we would have sorted them by now
					//so we have a bunch of shells, that failed to be sewn and are not solids
					//if we make them into a solid they will have free edges so will not be closed
					//best we can is try and fix them
					TopoDS_Solid solid = solidmaker.Solid();

					if (!solid.IsNull())
					{


						if (BRepCheck_Analyzer(solid, Standard_True).IsValid() == Standard_False)
						{
							//try and fix if we can
							ShapeFix_Solid fixer(solid);
							fixer.LimitTolerance(closedShell->Model->ModelFactors->Precision);
							fixer.SetMaxTolerance(closedShell->Model->ModelFactors->OneMilliMeter);
							fixer.SetMinTolerance(closedShell->Model->ModelFactors->Precision);
							fixer.SetPrecision(closedShell->Model->ModelFactors->Precision);


							Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(XbimGeometryCreator::BooleanTimeOut);
							if (fixer.Perform(pi))
							{
								solidMap.Add(fixer.Solid());
							}
							else
								XbimGeometryCreator::LogWarning(logger, closedShell, "Failed to create a valid solid from an IfcClosedShell");
						}
						else
						{
							solidMap.Add(solid);
						}

					}
				}
				if (solidMap.Size() > 0)
				{

					pCompound->Nullify();
					b.MakeCompound(*pCompound);

					for (int isolid = 1; isolid <= solidMap.Extent(); ++isolid)
					{
						TopoDS_Shape solid = solidMap.FindKey(isolid);
						//double oneCubicMillimetre = Math::Pow(closedShell->Model->ModelFactors->OneMilliMeter, 3);
						/*GProp_GProps gProps;
						try
						{
							BRepGProp::VolumeProperties(solid, gProps);
							double volume = gProps.Mass();
							if (volume < 0) solid.Reverse();*/
						b.Add(*pCompound, solid);
						/*}
						catch (...)
						{
							BRepTools::Write(solid, "c:/tmp/1");
						}*/

					}

					//volume = Math::Abs(volume);
					//if (volume != 0 && volume < oneCubicMillimetre) //sometimes zero volume is just a badly defined shape so let it through maybe
					//{
					//	XbimGeometryCreator::LogWarning(logger, closedShell, "Very small closed IfcClosedShell has been ignored");
					//	pCompound->Nullify();
					//	pCompound = nullptr;
					//}
				}
				else
				{
					XbimGeometryCreator::LogWarning(logger, closedShell, "Failed to create a valid solid from an IfcClosedShell");
					pCompound->Nullify();
					pCompound = nullptr;
				}
			}
		}
		void XbimCompound::Init(IIfcOpenShell^ openShell, ILogger^ logger)
		{
			Init((IIfcConnectedFaceSet^)openShell, logger);
		}
		bool XbimCompound::Sew()
		{

			if (!IsValid || IsSewn)
				return true;
			long tally = 0;
			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE); expl.More(); expl.Next())
			{
				tally++;
				if (tally > MaxFacesToSew) //give up if too many
					return false;
			}

			BRep_Builder builder;
			TopoDS_Compound newCompound;
			builder.MakeCompound(newCompound);
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL); expl.More(); expl.Next())
			{
				BRepBuilderAPI_Sewing seamstress(_sewingTolerance);
				seamstress.Add(expl.Current());
				Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(XbimGeometryCreator::BooleanTimeOut);
				seamstress.Perform(pi);
				TopoDS_Shape result = seamstress.SewedShape();
				builder.Add(newCompound, result);
			}

			*pCompound = newCompound;



			_isSewn = true;
			GC::KeepAlive(this);
			return true;
		}

		double XbimCompound::Volume::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::VolumeProperties(*pCompound, gProps, Standard_True);
				GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;
		}
		//This method copes with faces that may be advanced as well as ordinary
		TopoDS_Shell XbimCompound::InitAdvancedFaces(IEnumerable<IIfcFace^>^ faces, ILogger^ logger)
		{
			try
			{
				ShapeFix_Edge edgeFixer;

				BRep_Builder builder;
				TopoDS_Shell shell;
				builder.MakeShell(shell);
				IIfcFace^ aFace = Enumerable::FirstOrDefault(faces);
				if (aFace == nullptr) return shell;
				IModel^ model = aFace->Model;
				ShapeFix_ShapeTolerance FTol;

				_sewingTolerance = model->ModelFactors->Precision;

				//collect all the geometry components				
				TColStd_DataMapOfIntegerListOfInteger topoFaces;
				TopTools_DataMapOfIntegerShape edgeCurves;
				TopTools_DataMapOfIntegerShape vertexGeometries;
				for each (IIfcFace ^ unloadedFace in  faces)
				{
					IIfcAdvancedFace^ advancedFace = dynamic_cast<IIfcAdvancedFace^>(model->Instances[unloadedFace->EntityLabel]); //improves performance and reduces memory load								
					TopoDS_Wire topoOuterLoop;
					TopTools_SequenceOfShape  topoInnerLoops;

					int numberOfBounds = advancedFace->Bounds->Count;

					//build the face surface
					TopoDS_Face topoAdvancedFace = gcnew XbimFace(advancedFace->FaceSurface, logger);

					if (topoAdvancedFace.IsNull())
					{
						XbimGeometryCreator::LogWarning(logger, advancedFace->FaceSurface, "Failed to create face surface #{0}", advancedFace->FaceSurface->EntityLabel);
						continue;
					}
					//get the face oroented correctly


					TopLoc_Location loc;
					Handle(Geom_Surface) geomSurface = BRep_Tool::Surface(topoAdvancedFace, loc);
					if (!advancedFace->SameSense)
						geomSurface->UReverse();
					BRepBuilderAPI_MakeFace faceMaker;
					faceMaker.Init(geomSurface, false, _sewingTolerance);
					topoAdvancedFace = faceMaker.Face();

					for each (IIfcFaceBound ^ ifcBound in advancedFace->Bounds) //build all the loops
					{
						TopTools_SequenceOfShape loopEdges;
						bool isOuter = (numberOfBounds == 1) || (dynamic_cast<IIfcFaceOuterBound^>(ifcBound) != nullptr);
						IIfcEdgeLoop^ edgeLoop = dynamic_cast<IIfcEdgeLoop^>(ifcBound->Bound);

						if (edgeLoop != nullptr) //they always should be
						{

							for each (IIfcOrientedEdge ^ orientedEdge in edgeLoop->EdgeList)
							{

								IIfcEdgeCurve^ edgeCurve = dynamic_cast<IIfcEdgeCurve^>(orientedEdge->EdgeElement);
								TopoDS_Edge topoEdgeCurve;

								if (!edgeCurves.IsBound(orientedEdge->EdgeElement->EntityLabel)) //need to create the raw edge curve
								{
									//find the topological vertexes
									TopoDS_Vertex startVertex;
									TopoDS_Vertex endVertex;
									//we use the vertices of the edge curve element

									if (!vertexGeometries.IsBound(orientedEdge->EdgeElement->EdgeStart->EntityLabel))
									{
										IIfcCartesianPoint^ edgeStart = ((IIfcCartesianPoint^)((IIfcVertexPoint^)edgeCurve->EdgeStart)->VertexGeometry);
										gp_Pnt startPnt(edgeStart->X, edgeStart->Y, (int)edgeStart->Dim == 3 ? edgeStart->Z : .0);
										builder.MakeVertex(startVertex, startPnt, Precision::Confusion());
										vertexGeometries.Bind(edgeCurve->EdgeStart->EntityLabel, startVertex);
									}
									else
										startVertex = TopoDS::Vertex(vertexGeometries.Find(edgeCurve->EdgeStart->EntityLabel));

									if (!vertexGeometries.IsBound(edgeCurve->EdgeEnd->EntityLabel))
									{
										IIfcCartesianPoint^ edgeEnd = ((IIfcCartesianPoint^)((IIfcVertexPoint^)edgeCurve->EdgeEnd)->VertexGeometry);
										gp_Pnt endPnt(edgeEnd->X, edgeEnd->Y, (int)edgeEnd->Dim == 3 ? edgeEnd->Z : .0);
										builder.MakeVertex(endVertex, endPnt, Precision::Confusion());
										vertexGeometries.Bind(edgeCurve->EdgeEnd->EntityLabel, endVertex);
									}
									else
										endVertex = TopoDS::Vertex(vertexGeometries.Find(edgeCurve->EdgeEnd->EntityLabel));

									Handle(Geom_Curve) sharedEdgeGeom = gcnew XbimCurve(edgeCurve->EdgeGeometry, logger);
									if (!edgeCurve->SameSense)
										sharedEdgeGeom->Reverse(); //reverse the geometry if the parameterisation is in a different direction to the edge start and end vertices

									double trim1Tolerance, trim2Tolerance, trimParam1, trimParam2;
									bool foundP1 = XbimCurve::LocatePointOnCurve(sharedEdgeGeom, startVertex, _sewingTolerance * 20, trimParam1, trim1Tolerance);
									bool foundP2 = XbimCurve::LocatePointOnCurve(sharedEdgeGeom, endVertex, _sewingTolerance * 20, trimParam2, trim2Tolerance);

									if (!foundP1) //assume before the start of the curve
									{
										XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to project vertex to edge geometry: #{0}, start point assumed", edgeCurve->EdgeGeometry->EntityLabel);
										trimParam1 = sharedEdgeGeom->FirstParameter();
										trim1Tolerance = _sewingTolerance;
									}
									if (!foundP2) //assume before the start of the curve
									{
										XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to project vertex to edge geometry: #{0}, start point assumed", edgeCurve->EdgeGeometry->EntityLabel);
										trimParam2 = sharedEdgeGeom->LastParameter();
										trim2Tolerance = _sewingTolerance;
									}
									//update the vertices tolerance if necessary
									double startVertexTolerance = BRep_Tool::Tolerance(startVertex);
									double endVertexTolerance = BRep_Tool::Tolerance(endVertex);
									if (trim1Tolerance > startVertexTolerance)
										builder.UpdateVertex(startVertex, trim1Tolerance);
									if (trim2Tolerance > endVertexTolerance)
										builder.UpdateVertex(endVertex, trim2Tolerance);

									BRepBuilderAPI_MakeEdge edgeMaker(sharedEdgeGeom, startVertex, endVertex, trimParam1, trimParam2);
									if (!edgeMaker.IsDone())
									{
										BRepBuilderAPI_EdgeError err = edgeMaker.Error();
										switch (err)
										{

										case BRepBuilderAPI_PointProjectionFailed:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: BRepBuilderAPI_PointProjectionFailed", edgeCurve->EntityLabel);
											break;
										case BRepBuilderAPI_ParameterOutOfRange:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: BRepBuilderAPI_ParameterOutOfRange", edgeCurve->EntityLabel);
											break;
										case BRepBuilderAPI_DifferentPointsOnClosedCurve:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: BRepBuilderAPI_DifferentPointsOnClosedCurve", edgeCurve->EntityLabel);
											break;
										case BRepBuilderAPI_PointWithInfiniteParameter:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: BRepBuilderAPI_PointWithInfiniteParameter", edgeCurve->EntityLabel);
											break;
										case BRepBuilderAPI_DifferentsPointAndParameter:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: BRepBuilderAPI_DifferentsPointAndParameter", edgeCurve->EntityLabel);
											break;
										case BRepBuilderAPI_LineThroughIdenticPoints:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: BRepBuilderAPI_LineThroughIdenticPoints", edgeCurve->EntityLabel);
											break;
										default:
											XbimGeometryCreator::LogWarning(logger, edgeCurve, "Failed to create edge #{0}: Unknown error", edgeCurve->EntityLabel);
											break;
										}
										continue; //carry on and try and ignore, no sensible fall back at this point
									}
									topoEdgeCurve = edgeMaker.Edge();

									edgeCurves.Bind(edgeCurve->EntityLabel, topoEdgeCurve);

								}
								else
									topoEdgeCurve = TopoDS::Edge(edgeCurves.Find(edgeCurve->EntityLabel));
								//record the face relation to this edge
								if (topoFaces.IsBound(edgeCurve->EntityLabel))
									topoFaces.ChangeFind(edgeCurve->EntityLabel).Append(advancedFace->EntityLabel);
								else
								{
									TColStd_ListOfInteger elist;
									elist.Append(advancedFace->EntityLabel);
									topoFaces.Bind(edgeCurve->EntityLabel, elist);
								}
								//reverse if necessary								
								if (!orientedEdge->Orientation)
									topoEdgeCurve = TopoDS::Edge(topoEdgeCurve.Reversed());

								if (ifcBound->Orientation)
								{
									bool addedPCurve = edgeFixer.FixAddPCurve(topoEdgeCurve, geomSurface, loc, false, _sewingTolerance); //add pcurves
									loopEdges.Append(topoEdgeCurve);
								}
								else
								{
									TopoDS_Edge revTopoEdgeCurve = TopoDS::Edge(topoEdgeCurve.Reversed());
									bool addedPCurve = edgeFixer.FixAddPCurve(revTopoEdgeCurve, geomSurface, loc, false, _sewingTolerance); //add pcurves	
									loopEdges.Append(revTopoEdgeCurve);
								}

							}
						}

						TopoDS_Wire loopWire;
						builder.MakeWire(loopWire);

						for (auto it = loopEdges.cbegin(); it != loopEdges.cend(); it++)
						{
							builder.Add(loopWire, *it);
						}
						ShapeFix_Wire wireFixer(loopWire, topoAdvancedFace, _sewingTolerance);
						if (wireFixer.FixReorder())
							loopWire = wireFixer.Wire();
						if (isOuter)
							topoOuterLoop = loopWire;
						else
						{
							topoInnerLoops.Append(loopWire);
						}
					}

					//if we have no outer loop defined, find the biggest
					if (topoOuterLoop.IsNull())
					{
						double area = 0;
						int foundIndex = -1;
						int idx = 0;
						for (auto it = topoInnerLoops.cbegin(); it != topoInnerLoops.cend(); ++it)
						{
							idx++;
							double loopArea = ShapeAnalysis::ContourArea(TopoDS::Wire(*it));
							if (loopArea > area)
							{
								topoOuterLoop = TopoDS::Wire(*it);
								area = loopArea;
								foundIndex = idx;
							}
						}
						if (foundIndex > 0)topoInnerLoops.Remove(foundIndex); //remove outer loop from inner loops
					}
					if (topoOuterLoop.IsNull())
					{
						//no bounded face
						continue;
					}

					faceMaker.Add(topoOuterLoop);
					if (topoInnerLoops.Size() > 0) //add any inner bounds
					{
						try
						{
							for (auto it = topoInnerLoops.cbegin(); it != topoInnerLoops.cend(); ++it)
							{

								TopoDS_Wire innerWire = TopoDS::Wire(*it);
								faceMaker.Add(innerWire);
								if (!faceMaker.IsDone())
									XbimGeometryCreator::LogWarning(logger, advancedFace, "Could not apply inner bound to face #{0}, it has been ignored", advancedFace->EntityLabel);
							}
							ShapeFix_Face fixFaceWire(topoAdvancedFace);

							fixFaceWire.FixOrientation();
							topoAdvancedFace = fixFaceWire.Face();
						}
						catch (Standard_Failure sf)
						{
							String^ err = gcnew String(sf.GetMessageString());
							XbimGeometryCreator::LogWarning(logger, advancedFace, "Could not apply  bound to face #{0}, it has been ignored", advancedFace->EntityLabel);
						}
					}
					else topoAdvancedFace = faceMaker.Face();
					ShapeFix_Face fixFace(topoAdvancedFace);
					fixFace.Perform();
					topoAdvancedFace = fixFace.Face();
					builder.Add(shell, topoAdvancedFace);
				}
				//check it is solid or not
				bool isSolid = true;
				for (auto it = topoFaces.cbegin(); it != topoFaces.cend(); ++it)
				{
					int refCount = it->Size();
					if (refCount != 2)isSolid = false;
				}

				BRepCheck_Shell checker(shell);
				BRepCheck_Status status = checker.Closed();
				if(status== BRepCheck_Status::BRepCheck_NoError)
				{
					if (!isSolid)
						XbimGeometryCreator::LogInfo(logger, aFace, "Internal issue. Ifc definition and OCC implementation seem at odds", aFace->EntityLabel);
					shell.Closed(true);
					shell.Checked(true);
				}
				return shell;
			}
			catch (Standard_Failure exc)
			{
				String^ err = gcnew String(exc.GetMessageString());
				throw gcnew Exception("General failure in advanced face building: " + err);
			}

		}

		//srl need to review this to use the normals provided in the ifc file
		void  XbimCompound::Init(IIfcTriangulatedFaceSet^ faceSet, ILogger^ logger)
		{
			BRep_Builder builder;
			TopoDS_Shell shell;
			builder.MakeShell(shell);
			int faceCount = 0;
			//create a list of all the vertices
			List<XbimVertex^>^ vertices = gcnew List<XbimVertex^>(Enumerable::Count(faceSet->Coordinates->CoordList));
			Dictionary<long long, XbimEdge^>^ edgeMap = gcnew Dictionary<long long, XbimEdge^>();

			for each (IEnumerable<Ifc4::MeasureResource::IfcLengthMeasure> ^ cp in faceSet->Coordinates->CoordList)
			{
				XbimTriplet<Ifc4::MeasureResource::IfcLengthMeasure> tpl = IEnumerableExtensions::AsTriplet<Ifc4::MeasureResource::IfcLengthMeasure>(cp);
				XbimVertex^ v = gcnew XbimVertex(tpl.A, tpl.B, tpl.C, _sewingTolerance);
				vertices->Add(v);
			}


			//make the triangles
			for each (IEnumerable<Ifc4::MeasureResource::IfcPositiveInteger> ^ indices in faceSet->CoordIndex)
			{
				try
				{
					XbimTriplet<Ifc4::MeasureResource::IfcPositiveInteger> tpl = IEnumerableExtensions::AsTriplet<Ifc4::MeasureResource::IfcPositiveInteger>(indices);

					TopoDS_Vertex v1; TopoDS_Vertex v2; TopoDS_Vertex v3;
					int i1 = (int)tpl.A - 1;
					int i2 = (int)tpl.B - 1;
					int i3 = (int)tpl.C - 1;
					if (i1 == i2 || i2 == i3 || i1 == i3)
						continue;//not a triangle
					v1 = vertices[i1];
					v2 = vertices[i2];
					v3 = vertices[i3];

					long long edgeKey1 = ((long long)i1 << 32) | i2;//put v1 in the high part of the key								
					long long edgeKey2 = ((long long)i2 << 32) | i3;///put v2 in the high part of the key				
					long long edgeKey3 = ((long long)i3 << 32) | i1;///put v3 in the high part of the key
					//do the reverse of the keys
					long long revEdgeKey1 = ((long long)i2 << 32) | i1;///put v1 in the high part of the key								
					long long revEdgeKey2 = ((long long)i3 << 32) | i2;///put v2 in the high part of the key				
					long long revEdgeKey3 = ((long long)i1 << 32) | i3;///put v3 in the high part of the key

					XbimEdge^ edge1;
					XbimEdge^ edge2;
					XbimEdge^ edge3;

					if (edgeMap->TryGetValue(revEdgeKey1, edge1)) //look for the opposite edge first
					{
						XbimEdge^ anoEdge1;
						if (!edgeMap->TryGetValue(edgeKey1, anoEdge1)) //if we don't find it create it, this means an edge i reffed twice in the same direction
						{
							edge1 = edge1->Reversed(); //make a reverse copy and add it to map
							edgeMap->Add(edgeKey1, edge1);//this will throw an exeption if the edge is in more than twice
						}

					}
					else // it might be in there but the wrong direction but we cannot deal with that now as it works all through the mesh, so assume it is ok just to add it
					{
						if (!edgeMap->TryGetValue(edgeKey1, edge1)) //if we don't find it create it, this means an edge i reffed twice in the same direction
						{// Make the edge						
							BRepLib_MakeEdge edgeMaker(TopoDS::Vertex(v1.Oriented(TopAbs_FORWARD)), TopoDS::Vertex(v2.Oriented(TopAbs_REVERSED)));
							if (edgeMaker.IsDone())
							{
								edge1 = gcnew XbimEdge(edgeMaker.Edge());
								edgeMap->Add(edgeKey1, edge1); //this will throw an exeption if the edge is in more than twice
							}
							else
								continue; //this triangle is not a triangle
						}
					}

					if (edgeMap->TryGetValue(revEdgeKey2, edge2)) //look for the opposite edge first
					{
						XbimEdge^ anoEdge2;
						if (!edgeMap->TryGetValue(edgeKey2, anoEdge2)) //if we don't find it create it, this means an edge i reffed twice in the same direction
						{
							edge2 = edge2->Reversed(); //make a reverse copy and add it to map
							edgeMap->Add(edgeKey2, edge2);//this will throw an exeption if the edge is in more than twice
						}
						/*else
							edge2 = anoEdge2->Reversed();*/
					}
					else // it might be in there but the wrong direction but we cannot deal with that now as it works all through the mesh, so assume it is ok just to add it
					{
						if (!edgeMap->TryGetValue(edgeKey2, edge2)) //if we don't find it create it, this means an edge i reffed twice in the same direction
						{
							// Make the edge						
							BRepLib_MakeEdge edgeMaker(TopoDS::Vertex(v2.Oriented(TopAbs_FORWARD)), TopoDS::Vertex(v3.Oriented(TopAbs_REVERSED)));
							if (edgeMaker.IsDone())
							{
								edge2 = gcnew XbimEdge(edgeMaker.Edge());
								edgeMap->Add(edgeKey2, edge2); //this will throw an exeption if the edge is in more than twice
							}
							else
								continue; //this triangle is not a triangle
						}
					}

					if (edgeMap->TryGetValue(revEdgeKey3, edge3)) //look for the opposite edge first
					{
						XbimEdge^ anoEdge3;
						if (!edgeMap->TryGetValue(edgeKey3, anoEdge3)) //if we don't find it create it, this means an edge i reffed twice in the same direction
						{
							edge3 = edge3->Reversed(); //make a reverse copy and add it to map
							edgeMap->Add(edgeKey3, edge3);//this will throw an exeption if the edge is in more than twice
						}
						/*else
							edge3 = anoEdge3->Reversed();*/
					}
					else // it might be in there but the wrong direction but we cannot deal with that now as it works all through the mesh, so assume it is ok just to add it
					{
						if (!edgeMap->TryGetValue(edgeKey3, edge3)) //if we don't find it create it, this means an edge i reffed twice in the same direction
						{
							// Make the edge						
							BRepLib_MakeEdge edgeMaker(TopoDS::Vertex(v3.Oriented(TopAbs_FORWARD)), TopoDS::Vertex(v1.Oriented(TopAbs_REVERSED)));
							if (edgeMaker.IsDone())
							{
								edge3 = gcnew XbimEdge(edgeMaker.Edge());
								edgeMap->Add(edgeKey3, edge3); //this will throw an exeption if the edge is in more than twice
							}
							else
								continue; //this triangle is not a triangle
						}
					}

					TopoDS_Wire wire;
					builder.MakeWire(wire);
					builder.Add(wire, edge1);
					builder.Add(wire, edge2);
					builder.Add(wire, edge3);

					BRepBuilderAPI_MakeFace faceMaker(wire, Standard_True);

					if (faceMaker.IsDone())
					{
						faceCount++;
						builder.Add(shell, faceMaker.Face());
					}
				}

				catch (const std::exception& exc)
				{
					String^ err = gcnew String(exc.what());
					XbimGeometryCreator::LogWarning(logger, faceSet, "Error build triangle in mesh. " + err);
				}
			}
			pCompound = new TopoDS_Compound();
			builder.MakeCompound(*pCompound);
			if (faceCount < MaxFacesToSew)
			{
				ShapeUpgrade_UnifySameDomain unifier(shell);
				unifier.SetAngularTolerance(0.00174533); //1 tenth of a degree
				unifier.SetLinearTolerance(_sewingTolerance);
				try
				{
					unifier.Build();
					builder.Add(*pCompound, unifier.Shape());
				}
				catch (...)
				{
					builder.Add(*pCompound, shell);
				}
			}
			else
				builder.Add(*pCompound, shell);
		}

		void XbimCompound::Init(IEnumerable<IIfcFace^>^ ifcFaces, IIfcRepresentationItem^ theItem, ILogger^ logger)
		{
			_sewingTolerance = theItem->Model->ModelFactors->Precision;

			bool useVertexMap = ifcFaces->GetType() != XbimPolygonalFaceSet::typeid;
			BRep_Builder builder;
			TopTools_DataMapOfIntegerShape vertexMap;
			if (Enumerable::Count(ifcFaces) == 1)
			{
				TopoDS_Shell shell;
				builder.MakeShell(shell);
				XbimFace^ face = gcnew XbimFace(Enumerable::First(ifcFaces), logger, useVertexMap, vertexMap);
				builder.Add(shell, face);
				pCompound = new TopoDS_Compound();
				builder.MakeCompound(*pCompound);
				builder.Add(*pCompound, shell);
			}
			else
			{
				BRepBuilderAPI_Sewing seamstress(_sewingTolerance);
				int allFaces = 0;

				for each (IIfcFace ^ ifcFace in ifcFaces)
				{
					XbimFace^ face = gcnew XbimFace(ifcFace, logger, useVertexMap, vertexMap);
					seamstress.Add(face);
					allFaces++;
				}
				Handle(XbimProgressIndicator) pi = new XbimProgressIndicator(XbimGeometryCreator::BooleanTimeOut);
				seamstress.SetMinTolerance(_sewingTolerance);
				seamstress.SetMaxTolerance(theItem->Model->ModelFactors->OneMilliMeter / 10);

				seamstress.Perform(pi);

				if (pi->TimedOut())
				{
					XbimGeometryCreator::LogWarning(logger, theItem, "Sewing of the Shell has exceed the timeout period of " + XbimGeometryCreator::BooleanTimeOut.ToString());
					return;
				}
				if (seamstress.SewedShape().IsNull())
				{
					XbimGeometryCreator::LogWarning(logger, theItem, "Sewing of the Shell has resulted in an empty shape ");
					return;
				}
				TopoDS_Shape result = seamstress.SewedShape();
				TopoDS_Compound unifiedCompound;
				builder.MakeCompound(unifiedCompound);
				//remove unnecesary faces, normally caused by triangulation, this improves boolean quality 
				if (allFaces > 6 && allFaces < MaxFacesToSew) //six is a cuboid no point in simplify that 
				{
					ShapeUpgrade_UnifySameDomain unifier(result);
					unifier.SetAngularTolerance(0.00174533); //1 tenth of a degree 
					unifier.SetLinearTolerance(_sewingTolerance);

					try
					{
						//sometimes unifier crashes 
						unifier.Build();
						builder.Add(unifiedCompound, unifier.Shape());

					}
					catch (...) //any failure 
					{
						//default to what we had 
						builder.Add(unifiedCompound, result);
					}
				}
				else
				{
					builder.Add(unifiedCompound, result);
				}

				pCompound = new TopoDS_Compound();
				*pCompound = unifiedCompound;
			}

			_isSewn = true;
		}


#pragma endregion


#pragma region Helpers



		XbimFace^ XbimCompound::BuildFace(List<Tuple<XbimWire^, IIfcPolyLoop^, bool>^>^ wires, IIfcFace^ owningFace, ILogger^ logger)
		{
			if (wires->Count == 0)
				return gcnew XbimFace();
			//IIfcCartesianPoint^ first = Enumerable::First(wires[0]->Item2->Polygon);
			//XbimPoint3D p(first->X, first->Y, first->Z);
			XbimVector3D n = XbimConvert::NewellsNormal(wires[0]->Item2);

			XbimFace^ face = gcnew XbimFace(wires[0]->Item1, true, owningFace->Model->ModelFactors->Precision, owningFace->EntityLabel, logger);
			if (n.DotProduct(face->Normal) <= 0) //they should be in the same direction
				face->Reverse();
			if (!wires[0]->Item3)
				face->Reverse();
			if (wires->Count == 1) return face; //take the first one

			for (int i = 1; i < wires->Count; i++) face->Add(wires[i]->Item1);
			IXbimWire^ outerBound = face->OuterBound;
			XbimVector3D faceNormal;// = outerBound->Normal;
			for each (Tuple<XbimWire^, IIfcPolyLoop^, bool> ^ wire in wires)
			{
				if (wire->Item1->Equals(outerBound))
				{
					faceNormal = XbimConvert::NewellsNormal(wire->Item2);
					if (!wire->Item3) faceNormal = faceNormal.Negated();
					break;
				}
			}

			if (face->OuterBound == nullptr) return face;

			face = gcnew XbimFace(outerBound, true, owningFace->Model->ModelFactors->Precision, owningFace->EntityLabel, logger); //create  a face with the right bound and direction

			for (int i = 0; i < wires->Count; i++)
			{
				XbimWire^ wire = wires[i]->Item1;
				if (!wire->Equals(outerBound))
				{
					XbimVector3D loopNormal = XbimConvert::NewellsNormal(wires[i]->Item2);
					if (!wires[i]->Item3)loopNormal = loopNormal.Negated();
					if (faceNormal.DotProduct(loopNormal) > 0) //they should be in opposite directions, so reverse
						wire->Reverse();
					if (!face->Add(wire))
						XbimGeometryCreator::LogWarning(logger, owningFace, "Failed to add an inner bound");
				}
			}
			return face;
		}


		//upgrades the result to the highest level and simplest object without loss of representation
		IXbimGeometryObject^ XbimCompound::Upgrade()
		{
			if (!IsValid) return this;
			//upgrade all shells to solids if we can
			BRep_Builder builder;
			TopoDS_Compound newCompound;
			builder.MakeCompound(newCompound);
			int count = 0;
			TopoDS_Shape lastAdded;
			for (TopExp_Explorer expl(*pCompound, TopAbs_SOLID); expl.More(); expl.Next())
			{
				lastAdded = expl.Current();
				builder.Add(newCompound, TopoDS::Solid(lastAdded));
				count++;
			}
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL, TopAbs_SOLID); expl.More(); expl.Next())
			{
				lastAdded = expl.Current();
				XbimShell^ shell = gcnew XbimShell(TopoDS::Shell(lastAdded));
				if (shell->IsClosed)
				{
					XbimSolid^ solid = (XbimSolid^)shell->MakeSolid();
					if (solid->IsValid)
						builder.Add(newCompound, solid);
					else
						builder.Add(newCompound, TopoDS::Shell(expl.Current()));
				}
				else
					builder.Add(newCompound, TopoDS::Shell(expl.Current()));
				count++;
			}
			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE, TopAbs_SHELL); expl.More(); expl.Next())
			{
				lastAdded = expl.Current();
				builder.Add(newCompound, TopoDS::Face(lastAdded));
				count++;
			}

			if (count == 1)
			{
				TopAbs_ShapeEnum st = lastAdded.ShapeType();
				if (st == TopAbs_SOLID)
					return gcnew XbimSolid(TopoDS::Solid(lastAdded));
				else if (st == TopAbs_SHELL)
					return gcnew XbimShell(TopoDS::Shell(lastAdded));
				else if (st == TopAbs_FACE)
					return gcnew XbimFace(TopoDS::Face(lastAdded));
			}
			return gcnew XbimCompound(newCompound, IsSewn, _sewingTolerance); //return the upgraded compound
		}

		//Makes all the faces in the compound in to a single shell, does not performa nay form of sewing
		IXbimShell^ XbimCompound::MakeShell()
		{
			if (Count == 1) //if we have one shell or a solid with just one shell then just return it
			{
				IXbimGeometryObject^ geom = this->First;

				XbimShell^ shell = dynamic_cast<XbimShell^>(geom);
				if (shell != nullptr) return shell;
				XbimSolid^ solid = dynamic_cast<XbimSolid^>(geom);
				if (solid != nullptr && solid->Shells->Count == 1)
					return solid->Shells->First;

			}
			//make all the faces in to one shell, this may be a topologically illegal object

			BRepPrim_Builder builder;
			TopoDS_Shell shell;
			builder.MakeShell(shell);
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(this, TopAbs_FACE, map);
			for (int i = 1; i <= map.Extent(); i++)
			{
				builder.AddShellFace(shell, TopoDS::Face(map(i)));
			}
			builder.CompleteShell(shell);
			return gcnew XbimShell(shell);

		}


		XbimCompound^ XbimCompound::Merge(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			TopoDS_Compound compound;
			BRep_Builder b;

			////first remove any that intersect as simple merging leads to illegal geometries.
			Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clusters = gcnew Dictionary<XbimSolid^, HashSet<XbimSolid^>^>();
			for each (IXbimSolid ^ solid in solids) //init all the clusters
			{
				XbimSolid^ solidToCheck = dynamic_cast<XbimSolid^>(solid);
				if (solidToCheck != nullptr)
					clusters[solidToCheck] = gcnew HashSet<XbimSolid^>();
			}
			if (clusters->Count == 0) return nullptr; //nothing to do


			b.MakeCompound(compound);
			if (clusters->Count == 1) //just one so return it
			{
				for each (XbimSolid ^ solid in clusters->Keys) //take the first one
				{
					b.Add(compound, solid);
					GC::KeepAlive(solid);
					return gcnew XbimCompound(compound, true, tolerance);
				}
			}
			for each (XbimSolid ^ solid in solids)
			{
				XbimSolid^ solidToCheck = dynamic_cast<XbimSolid^>(solid);
				if (solidToCheck != nullptr)
				{
					XbimRect3D bbToCheck = solidToCheck->BoundingBox;
					for each (KeyValuePair<XbimSolid^, HashSet<XbimSolid^>^> ^ cluster in clusters)
					{
						if (solidToCheck != cluster->Key && bbToCheck.Intersects(cluster->Key->BoundingBox))
							cluster->Value->Add(solidToCheck);
					}
				}
			}
			List<XbimSolid^>^ toMergeReduced = gcnew List<XbimSolid^>();
			Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clustersSparse = gcnew Dictionary<XbimSolid^, HashSet<XbimSolid^>^>();
			for each (KeyValuePair<XbimSolid^, HashSet<XbimSolid^>^> ^ cluster in clusters)
			{
				if (cluster->Value->Count > 0)
					clustersSparse->Add(cluster->Key, cluster->Value);
				else
					toMergeReduced->Add(cluster->Key); //record the ones to simply merge
			}
			clusters = nullptr;

			XbimSolid^ clusterAround = nullptr;
			for each (XbimSolid ^ fsolid in clustersSparse->Keys) //take the first one
			{
				clusterAround = fsolid;
				break;
			}

			while (clusterAround != nullptr)
			{
				HashSet<XbimSolid^>^ connected = gcnew HashSet<XbimSolid^>();
				XbimCompound::GetConnected(connected, clustersSparse, clusterAround);

				ShapeFix_ShapeTolerance fixTol;
				TopoDS_Shape unionedShape;
				for each (XbimSolid ^ toConnect in connected) //join up the connected
				{
					fixTol.SetTolerance(toConnect, tolerance);
					if (unionedShape.IsNull()) unionedShape = toConnect;
					else
					{
						try
						{
							BRepAlgoAPI_Fuse boolOp(unionedShape, toConnect);
							if (boolOp.HasErrors() == Standard_False)
								unionedShape = boolOp.Shape();
							else
								XbimGeometryCreator::LogWarning(logger, toConnect, "Boolean Union operation failed.");
						}
						catch (const std::exception& exc)
						{
							String^ err = gcnew String(exc.what());
							XbimGeometryCreator::LogWarning(logger, toConnect, "Boolean Union operation failed. " + err);
						}

					}
				}
				XbimSolidSet^ solidSet = gcnew XbimSolidSet(unionedShape);

				for each (XbimSolid ^ solid in solidSet) toMergeReduced->Add(solid);

				for each (XbimSolid ^ solid in connected) //remove what we have connected
					clustersSparse->Remove(solid);

				clusterAround = nullptr;
				for each (XbimSolid ^ fsolid in clustersSparse->Keys) //take the first one
				{
					clusterAround = fsolid;
					break;
				}
			}

			for each (XbimSolid ^ solid in toMergeReduced)
			{
				b.Add(compound, solid);
				GC::KeepAlive(solid);
			}

			return gcnew XbimCompound(compound, true, tolerance);

		}

		List<XbimSolid^>^ XbimCompound::GetDiscrete(List<XbimSolid^>^% toProcess)
		{
			List<XbimSolid^>^ discrete = gcnew List<XbimSolid^>(toProcess->Count);
			if (toProcess->Count > 0)
			{

				List<XbimSolid^>^ connected = gcnew List<XbimSolid^>(toProcess->Count);

				for each (XbimSolid ^ solid in toProcess)
				{
					if (discrete->Count == 0)
						discrete->Add(solid);
					else
					{
						XbimRect3D solidBB = solid->BoundingBox;
						bool isConnected = false;
						for each (XbimSolid ^ discreteSolid in discrete)
						{
							if (discreteSolid->BoundingBox.Intersects(solidBB))
							{
								connected->Add(solid);
								isConnected = true;
								break;
							}
						}
						if (!isConnected) discrete->Add(solid);
					}
				}
				toProcess = connected;
			}
			return discrete;
		}

		void  XbimCompound::GetConnected(HashSet<XbimSolid^>^ connected, Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clusters, XbimSolid^ clusterAround)
		{
			if (connected->Add(clusterAround))
			{
				for each (KeyValuePair<XbimSolid^, HashSet<XbimSolid^>^> ^ polysets in clusters)
				{
					if (!connected->Contains(polysets->Key) && !(polysets->Key == clusterAround) && polysets->Value->Contains(clusterAround))  //don't do the same one twice
					{
						GetConnected(connected, clusters, polysets->Key);
						for each (XbimSolid ^ poly in polysets->Value)
						{
							GetConnected(connected, clusters, poly);
						}
					}
				}
			}
		}

		///SRL Need to look at this and consider using DoBoolean framework
		XbimCompound^ XbimCompound::Cut(XbimCompound^ solids, double tolerance, ILogger^ logger)
		{
			if (!IsSewn) Sew();
			/*ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solids, tolerance);
			fixTol.SetTolerance(this, tolerance);*/
			String^ err = "";
			try
			{
				BRepAlgoAPI_Cut boolOp(this, solids);
				GC::KeepAlive(this);
				GC::KeepAlive(solids);

				if (boolOp.HasErrors() == Standard_False)
				{
					XbimCompound^ result = gcnew XbimCompound(TopoDS::Compound(boolOp.Shape()), true, tolerance);
					if (result->BoundingBox.Length() - this->BoundingBox.Length() > tolerance) //nonsense result forget it
						return this;
					else
						return result;
				}
			}
			catch (const std::exception& exc)
			{
				err = gcnew String(exc.what());
			}
			XbimGeometryCreator::LogWarning(logger, solids, "Boolean Cut operation failed. " + err);
			return XbimCompound::Empty;
		}

		XbimCompound^ XbimCompound::Union(XbimCompound^ solids, double tolerance, ILogger^ logger)
		{
			if (!IsSewn) Sew();
			/*ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solids, tolerance);
			fixTol.SetTolerance(this, tolerance);*/
			String^ err = "";
			try
			{
				BRepAlgoAPI_Fuse boolOp(this, solids);
				GC::KeepAlive(this);
				GC::KeepAlive(solids);
				if (boolOp.HasErrors() == Standard_False)
					return gcnew XbimCompound(TopoDS::Compound(boolOp.Shape()), true, tolerance);
			}
			catch (const std::exception& exc)
			{
				err = gcnew String(exc.what());
			}
			XbimGeometryCreator::LogWarning(logger, solids, "Boolean Union operation failed. " + err);
			return XbimCompound::Empty;
		}


		XbimCompound^ XbimCompound::Intersection(XbimCompound^ solids, double tolerance, ILogger^ logger)
		{
			if (!IsSewn) Sew();
			/*ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solids, tolerance);
			fixTol.SetTolerance(this, tolerance);*/
			String^ err = "";
			try
			{
				BRepAlgoAPI_Common boolOp(this, solids);
				GC::KeepAlive(this);
				GC::KeepAlive(solids);
				if (boolOp.HasErrors() == Standard_False)
					return gcnew XbimCompound(TopoDS::Compound(boolOp.Shape()), true, tolerance);
			}
			catch (const std::exception& exc)
			{
				err = gcnew String(exc.what());
			}
			XbimGeometryCreator::LogWarning(logger, solids, "Boolean Intersection operation failed. " + err);
			return XbimCompound::Empty;
		}

		IXbimSolidSet^ XbimCompound::Solids::get()
		{
			XbimSolidSet^ solids = gcnew XbimSolidSet();
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pCompound, TopAbs_SOLID, map);
			for (int i = 1; i <= map.Extent(); i++)
				solids->Add(gcnew XbimSolid(TopoDS::Solid(map(i))));
			GC::KeepAlive(this);
			return solids;
		}

		IXbimShellSet^ XbimCompound::Shells::get()
		{
			List<IXbimShell^>^ shells = gcnew List<IXbimShell^>();
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pCompound, TopAbs_SHELL, map);
			for (int i = 1; i <= map.Extent(); i++)
				shells->Add(gcnew XbimShell(TopoDS::Shell(map(i))));
			GC::KeepAlive(this);
			return gcnew XbimShellSet(shells);
		}

		IXbimFaceSet^ XbimCompound::Faces::get()
		{
			List<IXbimFace^>^ faces = gcnew List<IXbimFace^>();
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pCompound, TopAbs_FACE, map);
			for (int i = 1; i <= map.Extent(); i++)
				faces->Add(gcnew XbimFace(TopoDS::Face(map(i))));
			GC::KeepAlive(this);
			return gcnew XbimFaceSet(faces);
		}

		IXbimEdgeSet^ XbimCompound::Edges::get()
		{
			List<IXbimEdge^>^ edges = gcnew List<IXbimEdge^>();
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pCompound, TopAbs_EDGE, map);
			for (int i = 1; i <= map.Extent(); i++)
				edges->Add(gcnew XbimEdge(TopoDS::Edge(map(i))));
			GC::KeepAlive(this);
			return gcnew XbimEdgeSet(edges);
		}

		IXbimVertexSet^ XbimCompound::Vertices::get()
		{
			List<IXbimVertex^>^ vertices = gcnew List<IXbimVertex^>();
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pCompound, TopAbs_VERTEX, map);
			for (int i = 1; i <= map.Extent(); i++)
				vertices->Add(gcnew XbimVertex(TopoDS::Vertex(map(i))));
			GC::KeepAlive(this);
			return gcnew XbimVertexSet(vertices);
		}

		void XbimCompound::Add(IXbimGeometryObject^ geomObj)
		{
			XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
			if (occ != nullptr)
			{
				BRep_Builder builder;
				if (ptrContainer == IntPtr::Zero)
				{
					pCompound = new TopoDS_Compound();
					builder.MakeCompound(*pCompound);
				}
				builder.Add(*pCompound, occ);
			}
		}

		IXbimGeometryObjectSet^ XbimCompound::Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance, logger);
		}


		IXbimGeometryObjectSet^ XbimCompound::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance, logger);
		}


		IXbimGeometryObjectSet^ XbimCompound::Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance, logger);
		}

		IXbimGeometryObjectSet^ XbimCompound::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance, logger);
		}

		IXbimGeometryObjectSet^ XbimCompound::Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance, logger);
		}


		IXbimGeometryObjectSet^ XbimCompound::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance, logger);
		}
#pragma endregion


	}
}