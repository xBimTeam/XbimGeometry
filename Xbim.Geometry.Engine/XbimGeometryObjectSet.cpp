#include "XbimGeometryObjectSet.h"
#include "XbimSolidSet.h"
#include "XbimShellSet.h"
#include "XbimFaceSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimGeometryCreator.h"

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRep_Builder.hxx>

#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepTools.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <Message_ProgressIndicator.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepCheck_Analyzer.hxx>
using namespace System;
using namespace System::ComponentModel;
namespace Xbim
{
	namespace Geometry
	{
		XbimGeometryObjectSet::XbimGeometryObjectSet(IEnumerable<IXbimGeometryObject^>^ objects)
		{
			geometryObjects = gcnew List<IXbimGeometryObject^>(objects);
		}

		XbimGeometryObjectSet::XbimGeometryObjectSet()
		{
			geometryObjects = gcnew List<IXbimGeometryObject^>();
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::First::get()
		{
			if (geometryObjects->Count == 0) return nullptr;
			return geometryObjects[0];
		}

		int XbimGeometryObjectSet::Count::get()
		{
			return geometryObjects->Count;
		}

		IEnumerator<IXbimGeometryObject^>^ XbimGeometryObjectSet::GetEnumerator()
		{
			return geometryObjects->GetEnumerator();
		}

		bool XbimGeometryObjectSet::Sew()
		{
			bool sewn = false;
			for each (IXbimGeometryObject^ geom in geometryObjects)
			{
				XbimCompound^ comp = dynamic_cast<XbimCompound^>(geom);
				if (comp != nullptr)
					if (comp->Sew()) sewn = true;
			}
			return sewn;
		}

		IXbimGeometryObject ^ XbimGeometryObjectSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet();
			result->Tag = Tag;
			for each (IXbimGeometryObject^ geometryObject in geometryObjects)
			{
				XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
				XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
				if (occShape != nullptr)
					result->Add(occShape->Transformed(transformation));		
				else if (occSet != nullptr)
					result->Add(occSet->Transformed(transformation));			
			}
			return result;
		}

		IXbimGeometryObject ^ XbimGeometryObjectSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet();	
			result->Tag = Tag;
			for each (IXbimGeometryObject^ geometryObject in geometryObjects)
			{
				XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
				XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
				if (occShape != nullptr)
					result->Add(occShape->Moved(placement));
				else if (occSet != nullptr)
					result->Add(occSet->Moved(placement));
			}
			return result;
		}

		IXbimGeometryObject ^ XbimGeometryObjectSet::Moved(IIfcObjectPlacement ^ objectPlacement)
		{
			if (!IsValid) return this;
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet();
			result->Tag = Tag;
			for each (IXbimGeometryObject^ geometryObject in geometryObjects)
			{
				XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
				XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
				if (occShape != nullptr)
					result->Add(occShape->Moved(objectPlacement));
				else if (occSet != nullptr)
					result->Add(occSet->Moved(objectPlacement));
			}
			return result;
		}

		void XbimGeometryObjectSet::Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimGeometryObject^ geometryObject  in geometryObjects)
			{
				XbimSetObject^ objSet = dynamic_cast<XbimSetObject^>(geometryObject);
				XbimOccShape^ occObject = dynamic_cast<XbimOccShape^>(geometryObject);
				if (objSet != nullptr)
					objSet->Mesh(mesh, precision, deflection, angle);
				else if (occObject != nullptr)
					occObject->Mesh(mesh, precision, deflection, angle);
				else
					throw gcnew Exception("Unsupported geometry type cannot be meshed");
			}
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimGeometryObject^>^ result = gcnew List<IXbimGeometryObject^>(geometryObjects->Count);
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				result->Add(geomObj->Transform(matrix3D));
			}
			return gcnew XbimGeometryObjectSet(result);
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimGeometryObject^>^ result = gcnew List<IXbimGeometryObject^>(geometryObjects->Count);
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				result->Add(((XbimGeometryObject^)geomObj)->TransformShallow(matrix3D));
			}
			return gcnew XbimGeometryObjectSet(result);
		}

		XbimRect3D XbimGeometryObjectSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IXbimSolidSet^ XbimGeometryObjectSet::Solids::get()
		{
			XbimSolidSet^ solids = gcnew XbimSolidSet();
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimSolidSet^ ss = dynamic_cast<XbimSolidSet^>(geomObj);
				if (occ!=nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_SOLID, map);					
					for (int i = 1; i <= map.Extent(); i++)
						solids->Add(gcnew XbimSolid(TopoDS::Solid(map(i))));					
				}
				else if (ss!=nullptr)
				{
					for each (XbimSolid^ solid in ss)
					{
						solids->Add(ss);
					}		
				}

			}
			return solids;
		}

		IXbimShellSet^ XbimGeometryObjectSet::Shells::get()
		{
			List<IXbimShell^>^ shells = gcnew List<IXbimShell^>();
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimShellSet^ ss = dynamic_cast<XbimShellSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_SHELL, map);
					for (int i = 1; i <= map.Extent(); i++)
						shells->Add(gcnew XbimShell(TopoDS::Shell(map(i))));
				}
				else if (ss != nullptr)
				{
					for each (XbimShell^ shell in ss)
					{
						shells->Add(shell);
					}
				}
			}
			return gcnew XbimShellSet(shells);
			
		}

		IXbimFaceSet^ XbimGeometryObjectSet::Faces::get()
		{
			List<IXbimFace^>^ faces = gcnew List<IXbimFace^>();
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimFaceSet^ fs = dynamic_cast<XbimFaceSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_FACE, map);
					for (int i = 1; i <= map.Extent(); i++)
						faces->Add(gcnew XbimFace(TopoDS::Face(map(i))));
				}
				else if (fs != nullptr)
				{
					for each (XbimFace^ face in fs)
					{
						faces->Add(face);
					}
				}
			}
			return gcnew XbimFaceSet(faces);			
		}

		IXbimEdgeSet^ XbimGeometryObjectSet::Edges::get()
		{
			List<IXbimEdge^>^ edges = gcnew List<IXbimEdge^>();
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimEdgeSet^ es = dynamic_cast<XbimEdgeSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_EDGE, map);
					for (int i = 1; i <= map.Extent(); i++)
						edges->Add(gcnew XbimEdge(TopoDS::Edge(map(i))));
				}
				else if (es != nullptr)
				{
					for each (XbimEdge^ edge in es)
					{
						edges->Add(edge);
					}
				}
			}
			return gcnew XbimEdgeSet(edges);			
		}

		IXbimVertexSet^ XbimGeometryObjectSet::Vertices::get()
		{
			List<IXbimVertex^>^ vertices = gcnew List<IXbimVertex^>();
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimVertexSet^ vs = dynamic_cast<XbimVertexSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_VERTEX, map);
					for (int i = 1; i <= map.Extent(); i++)
						vertices->Add(gcnew XbimVertex(TopoDS::Vertex(map(i))));
				}
				else if (vs != nullptr)
				{
					for each (XbimVertex^ vertex in vs)
					{
						vertices->Add(vertex);
					}
				}
			}
			return gcnew XbimVertexSet(vertices);
		}


		bool XbimGeometryObjectSet::ParseGeometry(IEnumerable<IXbimGeometryObject^>^ geomObjects, TopTools_ListOfShape& toBeProcessed, Bnd_Array1OfBox& aBoxes,
			TopoDS_Shell& passThrough, double tolerance)
		{
			ShapeFix_ShapeTolerance FTol;
			BRep_Builder builder;
			TopoDS_Shell facesToBeCut;
			builder.MakeShell(facesToBeCut);
			bool hasFacesToProcess = false;
			bool hasObjectsToProcess = false;
			for each (IXbimGeometryObject^ iGeom in geomObjects)
			{			
				XbimShell^ shell = dynamic_cast<XbimShell^>(iGeom);
				XbimSolid^ solid = dynamic_cast<XbimSolid^>(iGeom);
				IEnumerable<IXbimGeometryObject^>^ geomSet = dynamic_cast<IEnumerable<IXbimGeometryObject^>^>(iGeom);
				XbimFace^ face = dynamic_cast<XbimFace^>(iGeom);
				
				if (solid != nullptr)
				{
				
					//FTol.SetTolerance(solid, tolerance); 			
					toBeProcessed.Append(solid);
					hasObjectsToProcess = true;
					
			//	BRepTools::Write((XbimOccShape^)solid, "c:\\tmp\\b");
					
				}
				else if (shell != nullptr)
				{
				//	BRepTools::Write((XbimOccShape^)solid, "c:\\tmp\\s");
					for (TopExp_Explorer expl(shell, TopAbs_FACE); expl.More(); expl.Next())
					{
						Bnd_Box bbFace;
						BRepBndLib::Add(expl.Current(), bbFace);
						for (int i = 1; i <= aBoxes.Length(); i++)
						{
							if (!bbFace.IsOut(aBoxes(i)))
							{
								//check if the face is not sitting on the cut
								builder.Add(facesToBeCut, expl.Current());
								hasFacesToProcess = true;
							}
							else
								builder.Add(passThrough, expl.Current());
						}
					}
				}
				else if (geomSet != nullptr)
				{
					if (ParseGeometry(geomSet, toBeProcessed, aBoxes, passThrough, tolerance)) //nothing to do so just return what we had
					{
						hasObjectsToProcess = true;
					}
				}
				else if (face != nullptr)
				{
					Bnd_Box bbFace;
					BRepBndLib::Add(face, bbFace);
					for (int i = 1; i <= aBoxes.Length(); i++)
					{
						if (!bbFace.IsOut(aBoxes(i)))
						{
							//check if the face is not sitting on the cut
							builder.Add(facesToBeCut, face);
							hasFacesToProcess = true;
						}
						else
							builder.Add(passThrough, face);
					}
				}
			}
			if (hasFacesToProcess)
			{
				hasObjectsToProcess = true;
				//sew the bits we are going to cut
				BRepBuilderAPI_Sewing seamstress(tolerance);
				seamstress.Add(facesToBeCut);
				seamstress.Perform();
				TopoDS_Shape shape = seamstress.SewedShape();
				/*FTol.SetTolerance(shape, tolerance * 10, TopAbs_VERTEX);
				FTol.SetTolerance(shape, tolerance, TopAbs_EDGE);
				FTol.SetTolerance(shape, tolerance / 10, TopAbs_FACE);*/
				toBeProcessed.Append(shape);
			}
			return hasObjectsToProcess;
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::PerformBoolean(BOPAlgo_Operation bop, IXbimGeometryObject^ geomObject, IXbimSolidSet^ solids, double tolerance)
		{

			List<IXbimGeometryObject^>^ geomObjects = gcnew List<IXbimGeometryObject^>();
			geomObjects->Add(geomObject);
			return PerformBoolean(bop, geomObjects, solids, tolerance);
		}

		String^ XbimGeometryObjectSet::ToBRep::get()
		{
			std::ostringstream oss;
			TopoDS_Compound comp = CreateCompound(geometryObjects);					
			BRepTools::Write(comp, oss);
			return gcnew String(oss.str().c_str());
		}

		TopoDS_Compound XbimGeometryObjectSet::CreateCompound(IEnumerable<IXbimGeometryObject^>^ geomObjects)
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimGeometryObject^ gObj in geomObjects)
			{
				XbimOccShape^ shape = dynamic_cast<XbimOccShape^>(gObj);
				IEnumerable<IXbimGeometryObject^>^ geomSet = dynamic_cast<IEnumerable<IXbimGeometryObject^>^>(gObj);
				if (shape != nullptr)
				{
					builder.Add(bodyCompound, (XbimOccShape^)gObj);
				}
				else if(geomSet!=nullptr)
					builder.Add(bodyCompound, CreateCompound(geomSet));
			}
			return bodyCompound;
		}


		IXbimGeometryObjectSet^ XbimGeometryObjectSet::PerformBoolean(BOPAlgo_Operation bop, IEnumerable<IXbimGeometryObject^>^ geomObjects, IXbimSolidSet^ solids, double tolerance)
		{
			String^ err = "";
			
			BRepAlgoAPI_BooleanOperation* pBuilder = nullptr;
			try
			{
				BRep_Builder builder;
				ShapeFix_ShapeTolerance FTol;
//tolerance *= 1.1;
				TopoDS_Compound comp = CreateCompound(geomObjects);
				Bnd_Box bodyBox;
				BRepBndLib::Add(comp, bodyBox);
				//get the subset of faces that intersect the openings
				//we are going to treat shells different from solids by only cutting those faces that intersect the cut
				TopoDS_Shell toBePassedThrough;
				TopoDS_Shell facesToCut;
				
				builder.MakeShell(facesToCut);
				builder.MakeShell(toBePassedThrough);
				TopTools_ListOfShape toBeProcessed;
				TopTools_ListOfShape cuttingObjects;
				int allBoxCount = 0;
				Bnd_Array1OfBox allBoxes(1, solids->Count);
				int i = 1;
				for each (XbimSolid^ solid in solids)
				{
					if (solid->IsValid)
					{
						/*char buff[256];
						sprintf(buff, "c:\\tmp\\O%d", i);
						BRepTools::Write(solid, buff);*/
											
						Bnd_Box box;
						BRepBndLib::Add(solid, box);
						allBoxes(i).SetGap(-tolerance * 2); //reduce to only catch faces that are inside tolerance and not sitting on the opening
						if (!bodyBox.IsOut(box)) //only try and cut it if it might intersect the body
						{
							
						//	FTol.SetTolerance(solid, tolerance, TopAbs_FACE | TopAbs_EDGE);
						/*	FTol.SetTolerance(solid, tolerance );
							FTol.SetTolerance(solid, tolerance / 10, TopAbs_FACE);*/
							cuttingObjects.Append(solid);
						}
						i++;
					}					
				}

				if (cuttingObjects.Extent() == 0)
				{					
					XbimGeometryCreator::LogInfo(solids, "Openings cannot be cut, none intersect with the body shape");
					return gcnew XbimGeometryObjectSet(geomObjects);
				}
				if (!ParseGeometry(geomObjects, toBeProcessed, allBoxes, toBePassedThrough, tolerance)) //nothing to do so just return what we had
					return gcnew XbimGeometryObjectSet(geomObjects);
				
				switch (bop) {
				case BOPAlgo_COMMON:
					pBuilder = new BRepAlgoAPI_Common();
					break;
				case BOPAlgo_FUSE:
					pBuilder = new BRepAlgoAPI_Fuse();
					break;
				case BOPAlgo_CUT:
					pBuilder = new BRepAlgoAPI_Cut();
					break;
				case BOPAlgo_SECTION:
					pBuilder = new BRepAlgoAPI_Section();
					break;
				default:
					break;
				}		
								
				pBuilder->SetArguments(toBeProcessed);
				pBuilder->SetTools(cuttingObjects);
			//	pBuilder->SetFuzzyValue(tolerance);
				pBuilder->SetNonDestructive(Standard_True);
				//pBuilder->RefineEdges();
				Handle(XbimProgressIndicator) aPI = new XbimProgressIndicator(XbimGeometryCreator::BooleanTimeOut);				
				pBuilder->SetProgressIndicator(aPI);
				pBuilder->Build();
				aPI->StopTimer();
				//XbimGeometryCreator::logger->FatalFormat("{0}", aPI->ElapsedTime());
				if (aPI->TimedOut())
				{
					XbimGeometryCreator::LogError(solids, "Boolean operation timed out after {0} seconds. Operation failed", (int)aPI->ElapsedTime());
				}
				if (pBuilder->ErrorStatus() == 0 && !pBuilder->Shape().IsNull())
				{
					
			   //	BRepTools::Write(pBuilder->Shape(), "c:\\tmp\\r");
					TopoDS_Compound occCompound;
					builder.MakeCompound(occCompound);
					if (BRepCheck_Analyzer(pBuilder->Shape(), Standard_True).IsValid() == Standard_True)
					{

						ShapeFix_Shape shapeFixer(pBuilder->Shape());
						shapeFixer.Perform();
						builder.Add(occCompound, shapeFixer.Shape());
					}
					else
						builder.Add(occCompound, pBuilder->Shape());
					XbimCompound^ compound = gcnew XbimCompound(occCompound, false, tolerance);

					if (bop != BOPAlgo_COMMON) //do not need to add these as they by definition do not intersect
					{
						TopExp_Explorer expl(toBePassedThrough, TopAbs_FACE);

						if (expl.More()) //only add if there are faces to consider
							compound->Add(gcnew XbimShell(toBePassedThrough));
					}
					
					XbimGeometryObjectSet^ geomObjs = gcnew XbimGeometryObjectSet();
					geomObjs->Add(compound);
					if (pBuilder != nullptr) delete pBuilder;

					//BRepTools::Write(toBePassedThrough, "d:\\tmp\\s");
					return geomObjs;

				} 
				GC::KeepAlive(solids);
				GC::KeepAlive(geomObjects);
			}
			catch (Standard_Failure e)
			{				
				err = "Standard Failure in Xbim.Geometry.Engine::PerformBoolean";
			}	
			catch (...) //catch all violations
			{
				err = "General Exception thrown in Xbim.Geometry.Engine::PerformBoolean";
			}
			XbimGeometryCreator::LogInfo(solids, "Boolean Cut failed. " + err);
			if (pBuilder != nullptr) delete pBuilder;
			return XbimGeometryObjectSet::Empty;
		}

		

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Cut(IXbimSolidSet^ solids, double tolerance)
		{						
			return PerformBoolean(BOPAlgo_CUT, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Cut(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return PerformBoolean(BOPAlgo_CUT, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance);
		}
		

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Union(IXbimSolidSet^ solids, double tolerance)
		{
			return PerformBoolean(BOPAlgo_FUSE, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Union(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return PerformBoolean(BOPAlgo_FUSE, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Intersection(IXbimSolidSet^ solids, double tolerance)
		{
			return PerformBoolean(BOPAlgo_COMMON, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Intersection(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return PerformBoolean(BOPAlgo_COMMON, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance);
		}
	}
}
