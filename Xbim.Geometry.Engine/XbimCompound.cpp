#include "XbimCompound.h"
#include "XbimGeometryCreator.h"
#include "XbimSolidSet.h"
#include "XbimGeomPrim.h"

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
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
#include <BRepBuilderAPI_Transform.hxx>
using namespace System;

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
				result->Add(gcnew XbimSolid(TopoDS::Solid(expl.Current())));
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL, TopAbs_SOLID); expl.More(); expl.Next())
				result->Add(gcnew XbimShell(TopoDS::Shell(expl.Current())));
			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE, TopAbs_SHELL); expl.More(); expl.Next())
				result->Add(gcnew XbimFace(TopoDS::Face(expl.Current())));
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
			return count;
		}

		IXbimGeometryObject^ XbimCompound::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Compound temp = TopoDS::Compound(gTran.Shape());
			return gcnew XbimCompound(temp, IsSewn, _sewingTolerance);
		}

		XbimRect3D XbimCompound::BoundingBox::get()
		{
			if (pCompound == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			BRepBndLib::Add(*pCompound, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		IXbimGeometryObject^ XbimCompound::First::get()
		{
			if (!IsValid) return nullptr;
			for (TopExp_Explorer expl(*pCompound, TopAbs_SOLID); expl.More(); expl.Next())
				return gcnew XbimSolid(TopoDS::Solid(expl.Current()));
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL, TopAbs_SOLID); expl.More(); expl.Next())
				return gcnew XbimShell(TopoDS::Shell(expl.Current()));
			for (TopExp_Explorer expl(*pCompound, TopAbs_FACE, TopAbs_SHELL); expl.More(); expl.Next())
				return gcnew XbimFace(TopoDS::Face(expl.Current()));
			return nullptr;
		}


		XbimCompound::XbimCompound(IfcConnectedFaceSet^ faceSet)
		{
			_sewingTolerance = faceSet->ModelOf->ModelFactors->PrecisionBoolean;
			Init(faceSet);
		}

		XbimCompound::XbimCompound(IfcShellBasedSurfaceModel^ sbsm)
		{
			_sewingTolerance = sbsm->ModelOf->ModelFactors->PrecisionBoolean;
			Init(sbsm);
		}

		XbimCompound::XbimCompound(IfcFaceBasedSurfaceModel^ fbsm)
		{
			_sewingTolerance = fbsm->ModelOf->ModelFactors->PrecisionBoolean;
			Init(fbsm);
		}

		XbimCompound::XbimCompound(IfcManifoldSolidBrep^ solid)
		{
			_sewingTolerance = solid->ModelOf->ModelFactors->PrecisionBoolean;
			Init(solid);
		}
		XbimCompound::XbimCompound(IfcFacetedBrep^ solid)
		{
			_sewingTolerance = solid->ModelOf->ModelFactors->PrecisionBoolean;
			Init(solid);
		}

		XbimCompound::XbimCompound(IfcFacetedBrepWithVoids^ solid)
		{
			_sewingTolerance = solid->ModelOf->ModelFactors->PrecisionBoolean;
			Init(solid);
		}

		XbimCompound::XbimCompound(IfcClosedShell^ solid)
		{
			_sewingTolerance = solid->ModelOf->ModelFactors->PrecisionBoolean;
			Init(solid);
		}

		XbimCompound::XbimCompound(const TopoDS_Compound& compound, bool sewn, double tolerance)
		{
			pCompound = new TopoDS_Compound();
			*pCompound = compound;
			_isSewn = sewn;
			_sewingTolerance = tolerance;
		}





#pragma region Initialisers

		void XbimCompound::Init(IfcFaceBasedSurfaceModel^ fbsm)
		{
			pCompound = new TopoDS_Compound();
			BRep_Builder builder;
			builder.MakeCompound(*pCompound);
			for each (IfcConnectedFaceSet^ faceSet in fbsm->FbsmFaces)
			{
				XbimCompound^ compound = gcnew XbimCompound(faceSet);
				for each (IXbimGeometryObject^ geom in compound)
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

		void XbimCompound::Init(IfcShellBasedSurfaceModel^ sbsm)
		{
			List<IfcFace^>^ faces = gcnew List<IfcFace^>();
			for each (IfcShell^ shell in sbsm->SbsmBoundary)
			{
				//get the faces
				IfcConnectedFaceSet^ faceSet = dynamic_cast<IfcConnectedFaceSet^>(shell);
				if (faceSet != nullptr) //this should never fail
					faces->AddRange(faceSet->CfsFaces);
			}
			Init(faces);
			for each (IXbimGeometryObject^ geomObj in this)
			{
				XbimShell^ shell = dynamic_cast<XbimShell^>(geomObj);
				if (shell!=nullptr) shell->Orientate();
			}
				
		}

		void XbimCompound::Init(IfcConnectedFaceSet^ faceSet)
		{
			if (faceSet->CfsFaces->Count == 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WC001: IfcConnectedFaceSet #{0}, is empty", faceSet->EntityLabel);
				return;
			}
			Init(faceSet->CfsFaces);
			for each (IXbimGeometryObject^ geomObj in this)
			{
				XbimShell^ shell = dynamic_cast<XbimShell^>(geomObj);
				if (shell != nullptr) shell->Orientate();
			}
		}


		void XbimCompound::Init(IfcManifoldSolidBrep^ solid)
		{
			IfcFacetedBrep^ facetedBrep = dynamic_cast<IfcFacetedBrep^>(solid);
			if (facetedBrep != nullptr) return Init(facetedBrep);
			throw gcnew NotImplementedException("Sub-Type of IfcManifoldSolidBrep is not implemented");
		}
		void XbimCompound::Init(IfcFacetedBrep^ solid)
		{
			IfcFacetedBrepWithVoids^ facetedBrepWithVoids = dynamic_cast<IfcFacetedBrepWithVoids^>(solid);
			if (facetedBrepWithVoids != nullptr) return Init(facetedBrepWithVoids);
			Init(solid->Outer);
		}


		void XbimCompound::Init(IfcFacetedBrepWithVoids^ brepWithVoids)
		{
			XbimCompound^ shapes = gcnew XbimCompound(brepWithVoids->Outer);
			XbimShell^ outerShell = (XbimShell^)shapes->MakeShell();
			if (!outerShell->IsClosed) //we have a shell that is not able to be made in to a solid
				XbimGeometryCreator::logger->ErrorFormat("ES004: An IfcClosedShell #{0} has been found that is not topologically correct. ", brepWithVoids->Outer->EntityLabel);
			BRepBuilderAPI_MakeSolid builder(outerShell);
			for each (IfcClosedShell^ ifcVoidShell in brepWithVoids->Voids)
			{
				XbimCompound^ voidShapes = gcnew XbimCompound(ifcVoidShell);
				XbimShell^ voidShell = (XbimShell^)voidShapes->MakeShell();
				if (!voidShell->IsClosed) //we have a shell that is not able to be made in to a solid
					XbimGeometryCreator::logger->ErrorFormat("ES004: An IfcClosedShell #{0} has been found that is not topologically correct. ", ifcVoidShell->EntityLabel);
				builder.Add(voidShell);
			}
			if (builder.IsDone())
			{
				pCompound= new TopoDS_Compound();
				BRep_Builder b;
				b.MakeCompound(*pCompound);
				b.Add(*pCompound, builder.Solid());
			}
			else
				XbimGeometryCreator::logger->ErrorFormat("ES003: An incorrectly defined IfcFacetedBrepWithVoids #{0} has been found, a correct shape could not be built and it has been ignored", brepWithVoids->EntityLabel);
		}

		void XbimCompound::Init(IfcClosedShell^ closedShell)
		{
			Init((IfcConnectedFaceSet^)closedShell);
		}

		void XbimCompound::Sew()
		{
			if(!IsValid || IsSewn) 
				return;
			BRep_Builder builder;
			TopoDS_Compound newCompound;
			builder.MakeCompound(newCompound);
			for (TopExp_Explorer expl(*pCompound, TopAbs_SHELL); expl.More(); expl.Next())
			{
				BRepBuilderAPI_Sewing seamstress(_sewingTolerance);
				seamstress.Add(expl.Current());
				seamstress.Perform();
				TopoDS_Shape result = seamstress.SewedShape();
				builder.Add(newCompound, result);
			}		
			*pCompound = newCompound;
			_isSewn = true;
		}

		void XbimCompound::Init(IEnumerable<IfcFace^>^ faces)
		{
			double tolerance;
			
			for each (IfcFace^ face in faces)
			{ 
				tolerance = face->ModelOf->ModelFactors->Precision;
				_sewingTolerance = face->ModelOf->ModelFactors->PrecisionBoolean;
				break;
			}
			
			
			BRep_Builder builder;
			TopoDS_Shell shell;
			builder.MakeShell(shell);
			TopTools_DataMapOfIntegerShape vertexStore;
			for each (IfcFace^ fc in  faces)
			{

				List<XbimWire^>^ loops = gcnew List<XbimWire^>();

				for each (IfcFaceBound^ bound in fc->Bounds) //build all the loops
				{
					if (!dynamic_cast<IfcPolyLoop^>(bound->Bound) || ((IfcPolyLoop^)bound->Bound)->Polygon->Count < 3) continue;//skip non-polygonal faces
					IfcPolyLoop^polyLoop = (IfcPolyLoop^)bound->Bound;
					bool is3D = (polyLoop->Polygon[0]->Dim == 3);
					BRepBuilderAPI_MakePolygon polyMaker;
					for each (IfcCartesianPoint^ p in polyLoop->Polygon) //add all the points into unique collection
					{
						TopoDS_Vertex v;
						if (!vertexStore.IsBound(p->EntityLabel))
						{
							builder.MakeVertex(v, gp_Pnt(p->X, p->Y, is3D ? p->Z : 0), tolerance);
							vertexStore.Bind(p->EntityLabel, v);
						}
						else
							v = TopoDS::Vertex(vertexStore.Find(p->EntityLabel));
						polyMaker.Add(v);
					}
					
					if (polyMaker.IsDone())
					{
						polyMaker.Close();
						XbimWire^ loop = gcnew XbimWire(polyMaker.Wire());
						if (loop->IsValid)
						{
							if (!bound->Orientation) loop->Reverse();
							loops->Add(loop);
						}
					}
				
				}
				XbimFace^ face = BuildFace(loops, fc->EntityLabel);
				if (face->IsValid)
				{
					builder.Add(shell, face);
				}
				else
					XbimGeometryCreator::logger->WarnFormat("WC002: Incorrectly defined IfcFace #{0}", fc->EntityLabel);
			}
		
			pCompound = new TopoDS_Compound();
			builder.MakeCompound(*pCompound);
			builder.Add(*pCompound, shell);
			
		}


#pragma endregion


#pragma region Helpers


		
		XbimFace^ XbimCompound::BuildFace(List<XbimWire^>^ wires, int label)
		{
			if (wires->Count == 0) return gcnew XbimFace();
			XbimFace^ face = gcnew XbimFace(wires[0]); //take the first one
			if (wires->Count == 1) return face;
			for (int i = 1; i < wires->Count; i++) face->Add(wires[i]);
			IXbimWire^ outerBound = face->OuterBound;
			face = gcnew XbimFace(outerBound); //create  a face with the right bound and direction
			XbimVector3D faceNormal = outerBound->Normal;

			for each (XbimWire^ wire in wires)
			{
				if (!wire->Equals(outerBound))
				{
					XbimVector3D loopNormal = wire->Normal;
					if (faceNormal.DotProduct(loopNormal) > 0) //they should be in opposite directions, so reverse
						wire->Reverse();
					if (!face->Add(wire))
						XbimGeometryCreator::logger->WarnFormat("WC003: Failed to add an inner bound to IfcFace #{0}", label);
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
						builder.Add(newCompound,  solid);
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


		XbimCompound^ XbimCompound::Merge(IXbimSolidSet^ solids, double tolerance)
		{
			//first remove any that intersect as simple merging leads to illegal geometries.
			Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clusters = gcnew Dictionary<XbimSolid^, HashSet<XbimSolid^>^>();
			for each (IXbimSolid^ solid in solids) //init all the clusters
			{
				XbimSolid^ solidToCheck = dynamic_cast<XbimSolid^>(solid);
				if (solidToCheck != nullptr)
					clusters[solidToCheck] = gcnew HashSet<XbimSolid^>();
			}
			if (clusters->Count == 0) return nullptr; //nothing to do

			TopoDS_Compound compound;
			BRep_Builder b;
			b.MakeCompound(compound);
			if (clusters->Count == 1 ) //just one so return it
			{
				for each(XbimSolid^ solid in clusters->Keys) //take the first one
				{
					b.Add(compound, solid);
					GC::KeepAlive(solid);
					return gcnew XbimCompound(compound, true, tolerance);
				}
			}
			for each (XbimSolid^ solid in solids)
			{
				XbimSolid^ solidToCheck = dynamic_cast<XbimSolid^>(solid);
				if (solidToCheck != nullptr)
				{
					XbimRect3D bbToCheck = solidToCheck->BoundingBox;
					for each (KeyValuePair<XbimSolid^, HashSet<XbimSolid^>^>^ cluster in clusters)
					{
						if (solidToCheck != cluster->Key && bbToCheck.Intersects(cluster->Key->BoundingBox))
							cluster->Value->Add(solidToCheck);
					}
				}
			}
			List<XbimSolid^>^ toMergeReduced = gcnew List<XbimSolid^>();
			Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clustersSparse = gcnew Dictionary<XbimSolid^, HashSet<XbimSolid^>^>();
			for each (KeyValuePair<XbimSolid^, HashSet<XbimSolid^>^>^ cluster in clusters)
			{
				if (cluster->Value->Count>0)
					clustersSparse->Add(cluster->Key, cluster->Value);
				else
					toMergeReduced->Add(cluster->Key); //record the ones to simply merge
			}
			clusters = nullptr;

			XbimSolid^ clusterAround = nullptr;
			for each(XbimSolid^ fsolid in clustersSparse->Keys) //take the first one
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
				for each (XbimSolid^ toConnect in connected) //join up the connected
				{
					fixTol.SetTolerance(toConnect, tolerance);
					if (unionedShape.IsNull()) unionedShape = toConnect;
					else
					{
						String^ err = "";
						try
						{
							BRepAlgoAPI_Fuse boolOp(unionedShape, toConnect);
							if (boolOp.ErrorStatus() == 0)
								unionedShape = boolOp.Shape();
							else
								XbimGeometryCreator::logger->WarnFormat("WC004: Boolean Union operation failed." );
						}
						catch (Standard_Failure e)
						{
							err = gcnew String(Standard_Failure::Caught()->GetMessageString());
							XbimGeometryCreator::logger->WarnFormat("WC005: Boolean Union operation failed. " + err);
						}
						
					}
				}
				XbimSolidSet^ solidSet = gcnew XbimSolidSet(unionedShape);

				for each (XbimSolid^ solid in solidSet) toMergeReduced->Add(solid);
				
				for each (XbimSolid^ solid in connected) //remove what we have connected
					clustersSparse->Remove(solid);

				clusterAround = nullptr;
				for each(XbimSolid^ fsolid in clustersSparse->Keys) //take the first one
				{
					clusterAround = fsolid;
					break;
				}
			}

			for each (XbimSolid^ solid in toMergeReduced)
			{
				b.Add(compound, solid);
				GC::KeepAlive(solid);
			}
			
			return gcnew XbimCompound(compound, true, tolerance);

		}

		void  XbimCompound::GetConnected(HashSet<XbimSolid^>^ connected, Dictionary<XbimSolid^, HashSet<XbimSolid^>^>^ clusters, XbimSolid^ clusterAround)
		{
			if (connected->Add(clusterAround))
			{
				for each (KeyValuePair<XbimSolid^, HashSet<XbimSolid^>^>^ polysets in clusters)
				{
					if (!connected->Contains(polysets->Key) && !(polysets->Key == clusterAround) && polysets->Value->Contains(clusterAround))  //don't do the same one twice
					{
						GetConnected(connected, clusters, polysets->Key);
						for each (XbimSolid^ poly in polysets->Value)
						{
							GetConnected(connected, clusters, poly);
						}
					}
				}
			}
		}

		XbimCompound^ XbimCompound::Cut(XbimCompound^ solids, double tolerance)
		{
			if (!IsSewn) Sew();
			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solids, tolerance);
			fixTol.SetTolerance(this, tolerance);
			String^ err = "";
			try
			{
				BRepAlgoAPI_Cut boolOp(this, solids);
				GC::KeepAlive(this);
				GC::KeepAlive(solids);
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimCompound(TopoDS::Compound(boolOp.Shape()),true,tolerance);
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::logger->WarnFormat("WC001: Boolean Cut operation failed. " + err);
			return XbimCompound::Empty;
		}

		XbimCompound^ XbimCompound::Union(XbimCompound^ solids, double tolerance)
		{
			if (!IsSewn) Sew();
			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solids, tolerance);
			fixTol.SetTolerance(this, tolerance);
			String^ err = "";
			try
			{
				BRepAlgoAPI_Fuse boolOp(this, solids);
				GC::KeepAlive(this);
				GC::KeepAlive(solids);
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimCompound(TopoDS::Compound(boolOp.Shape()),true,tolerance);
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::logger->WarnFormat("WC002: Boolean Union operation failed. " + err);
			return XbimCompound::Empty;
		}

		XbimCompound^ XbimCompound::Intersection(XbimCompound^ solids, double tolerance)
		{
			if (!IsSewn) Sew();
			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(solids, tolerance);
			fixTol.SetTolerance(this, tolerance);
			String^ err = "";
			try
			{
				BRepAlgoAPI_Common boolOp(this, solids);
				GC::KeepAlive(this);
				GC::KeepAlive(solids);
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimCompound(TopoDS::Compound(boolOp.Shape()),true,tolerance);
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::logger->WarnFormat("WC003: Boolean Intersection operation failed. " + err);
			return XbimCompound::Empty;
		}
#pragma endregion


	}
}