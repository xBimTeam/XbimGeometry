#include "XbimOccShape.h"
#include "XbimFaceSet.h"
#include "XbimShell.h"
#include "XbimSolid.h"
#include "XbimCompound.h"
#include "XbimPoint3DWithTolerance.h"
#include "XbimConvert.h"
#include <BRepCheck_Analyzer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <TShort_Array1OfShortReal.hxx>
#include <BRep_Tool.hxx>
#include <Poly.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Quaternion.hxx>
#include "XbimWire.h"
#include <TopExp.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Line.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_Transform.hxx>
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace Xbim::Tessellator;
//
//IMPLEMENT_STANDARD_HANDLE(XbimProgressIndicator, Message_ProgressIndicator)
//IMPLEMENT_STANDARD_RTTIEXT(XbimProgressIndicator, Message_ProgressIndicator)


XbimProgressIndicator::XbimProgressIndicator(Standard_Real maxDurationSeconds, bool startTimer) :
	Message_ProgressIndicator()
{
	maxRunDuration = maxDurationSeconds;
	if (startTimer) StartTimer();
}

Standard_Boolean XbimProgressIndicator::UserBreak()
{
	
	if (ElapsedTime() > maxRunDuration)
	{
		StopTimer();
		timedOut = true;
		return true;
	}
	else
		return false;
}



namespace Xbim
{
	namespace Geometry
	{
		XbimOccShape::XbimOccShape()
		{
		}



		void XbimOccShape::WriteTriangulation(TextWriter^ textWriter, double tolerance, double deflection, double angle)
		{

			if (!IsValid) return;
			XbimFaceSet^ faces = gcnew XbimFaceSet(this);

			if (faces->Count == 0) return;

			Monitor::Enter(this);
			try
			{
				BRepMesh_IncrementalMesh incrementalMesh(this, deflection, Standard_False, angle); //triangulate the first time				
			}
			finally
			{
				Monitor::Exit(this);
			}

			Dictionary<XbimPoint3DWithTolerance^, size_t>^ pointMap = gcnew Dictionary<XbimPoint3DWithTolerance^, size_t>();
			List<List<size_t>^>^ pointLookup = gcnew List<List<size_t>^>(faces->Count);
			List<XbimPoint3D>^ points = gcnew List<XbimPoint3D>(faces->Count * 5);;

			Dictionary<XbimPoint3DWithTolerance^, size_t>^ normalMap = gcnew Dictionary<XbimPoint3DWithTolerance^, size_t>();
			List<List<size_t>^>^ normalLookup = gcnew List<List<size_t>^>(faces->Count);
			List<XbimVector3D>^ normals = gcnew List<XbimVector3D>(faces->Count * 4);
			List<XbimFace^>^ writtenFaces = gcnew List<XbimFace^>(faces->Count);
			//First write out all the vertices
			int faceIndex = 0;
			int triangleCount = 0;
			for each (XbimFace^ face in faces)
			{
				TopLoc_Location loc;
				const Handle(Poly_Triangulation)& mesh = BRep_Tool::Triangulation(face, loc);
				if (mesh.IsNull())
					continue;
				gp_Trsf transform = loc.Transformation();
				gp_Quaternion quaternion = transform.GetRotation();
				const TColgp_Array1OfPnt & nodes = mesh->Nodes();
				triangleCount += mesh->NbTriangles();
				bool faceReversed = face->IsReversed;
				bool isPolygonal = face->IsPolygonal;
				pointLookup->Add(gcnew List<size_t>(mesh->NbNodes()));
				List<size_t>^ norms;
				if (!isPolygonal)
				{
					Poly::ComputeNormals(mesh); //we need the normals
					norms = gcnew List<size_t>(mesh->NbNodes());
					for (Standard_Integer i = 1; i <= mesh->NbNodes() * 3; i += 3) //visit each node
					{
						gp_Dir dir(mesh->Normals().Value(i), mesh->Normals().Value(i + 1), mesh->Normals().Value(i + 2));
						if (faceReversed) dir.Reverse();
						size_t index;
						dir = quaternion.Multiply(dir);
						XbimPoint3DWithTolerance^ n = gcnew XbimPoint3DWithTolerance(dir.X(), dir.Y(), dir.Z(), tolerance);
						if (!normalMap->TryGetValue(n, index))
						{
							index = normalMap->Count;
							normalMap->Add(n, index);
							normals->Add(XbimVector3D(dir.X(), dir.Y(), dir.Z()));
						}
						norms->Add(index);
					}
				}
				else
				{
					norms = gcnew List<size_t>(1);
					size_t index;
					XbimPoint3DWithTolerance^ n = gcnew XbimPoint3DWithTolerance(face->Normal.X, face->Normal.Y, face->Normal.Z, tolerance);
					if (!normalMap->TryGetValue(n, index))
					{
						index = normalMap->Count;
						normalMap->Add(n, index);
						normals->Add(XbimVector3D(n->X, n->Y, n->Z));
					}
					norms->Add(index);
				}
				normalLookup->Add(norms);
				for (Standard_Integer i = 1; i <= mesh->NbNodes(); i++) //visit each node for vertices
				{
					gp_XYZ p = nodes.Value(i).XYZ();
					transform.Transforms(p);
					size_t index;
					XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
					if (!pointMap->TryGetValue(pt, index))
					{
						index = pointMap->Count;
						pointMap->Add(pt, index);
						points->Add(pt->VertexGeometry);
					}
					pointLookup[faceIndex]->Add(index);
				}
				writtenFaces->Add(face);
				faceIndex++;
			}
			// Write out header
			textWriter->WriteLine(String::Format("P {0} {1} {2} {3} {4}", 1, points->Count, faces->Count, triangleCount, normals->Count));
			//write out vertices and normals  
			textWriter->Write("V");
			for each (XbimPoint3D p in points) textWriter->Write(String::Format(" {0},{1},{2}", p.X, p.Y, p.Z));
			textWriter->WriteLine();
			textWriter->Write("N");
			for each (XbimVector3D n in normals) textWriter->Write(String::Format(" {0},{1},{2}", n.X, n.Y, n.Z));
			textWriter->WriteLine();

			//now write out the faces
			faceIndex = 0;
			for each (XbimFace^ face in writtenFaces)
			{
				bool isPlanar = face->IsPlanar;
				List<size_t>^ norms = normalLookup[faceIndex];
				textWriter->Write("T");
				List<size_t>^ nodeLookup = pointLookup[faceIndex];
				TopLoc_Location loc;
				const Handle(Poly_Triangulation)& mesh = BRep_Tool::Triangulation(face, loc);
				const TColgp_Array1OfPnt & nodes = mesh->Nodes();
				const Poly_Array1OfTriangle& triangles = mesh->Triangles();
				Standard_Integer nbTriangles = mesh->NbTriangles();
				bool faceReversed = face->IsReversed;
				Standard_Integer t[3];
				for (Standard_Integer i = 1; i <= nbTriangles; i++) //add each triangle as a face
				{
					if (faceReversed) //get nodes in the correct order of triangulation
						triangles(i).Get(t[2], t[1], t[0]);
					else
						triangles(i).Get(t[0], t[1], t[2]);
					if (isPlanar)
						if (i == 1)
							textWriter->Write(String::Format(" {0}/{3},{1},{2}", nodeLookup[t[0] - 1], nodeLookup[t[1] - 1], nodeLookup[t[2] - 1], norms[0]));
						else
							textWriter->Write(String::Format(" {0},{1},{2}", nodeLookup[t[0] - 1], nodeLookup[t[1] - 1], nodeLookup[t[2] - 1]));
					else //need to write every one
						textWriter->Write(String::Format(" {0}/{3},{1}/{4},{2}/{5}", nodeLookup[t[0] - 1], nodeLookup[t[1] - 1], nodeLookup[t[2] - 1], norms[t[0] - 1], norms[t[1] - 1], norms[t[2] - 1]));
				}
				faceIndex++;
				textWriter->WriteLine();
			}			
			textWriter->Flush();
			GC::KeepAlive(this);
		}

		void XbimOccShape::WriteTriangulation(IXbimMeshReceiver^ meshReceiver, double tolerance, double deflection, double angle)
		{
			if (!IsValid) return;
			if (meshReceiver == nullptr)
			{
				try
				{
					Monitor::Enter(this);
					BRepMesh_IncrementalMesh incrementalMesh(this, deflection, Standard_False, angle); //triangulate the first time	
				}
				finally
				{
					Monitor::Exit(this);
				}
				return;
			}
			TopTools_IndexedMapOfShape faceMap;
			TopoDS_Shape shape = this; //hold on to it
			TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
			int faceCount = faceMap.Extent();
			if (faceCount == 0) return;		
			array<bool>^ hasSeams = gcnew array<bool>(faceCount);
			for (int f = 0; f < faceMap.Extent(); f++)
			{
				TopTools_IndexedMapOfShape edgeMap;
				TopExp::MapShapes(faceMap(f+1), TopAbs_EDGE, edgeMap);
				hasSeams[f] = false;
				//deal with seams
				for (Standard_Integer i = 1; i <= edgeMap.Extent(); i++)
				{				
					//find any seams					
					hasSeams[f] = (BRep_Tool::IsClosed(edgeMap(i)) == Standard_True); //just check a seam once
					if (hasSeams[f]) break; //this face has a seam no need to do more
				}
			}
			
			BRepMesh_IncrementalMesh incrementalMesh(this, deflection, Standard_False, angle); //triangulate the first time		

			
			for (int f = 1; f <= faceMap.Extent(); f++)
			{
				const TopoDS_Face& face = TopoDS::Face(faceMap(f));
				int faceId = meshReceiver->AddFace();
				bool faceReversed = (face.Orientation() == TopAbs_REVERSED);

				TopLoc_Location loc;
				const Handle(Poly_Triangulation)& mesh = BRep_Tool::Triangulation(face, loc);
				if (mesh.IsNull())
					continue;
				//check if we have a seam
				bool hasSeam = hasSeams[f - 1];
				gp_Trsf transform = loc.Transformation();
				gp_Quaternion quaternion = transform.GetRotation();
				const TColgp_Array1OfPnt & nodes = mesh->Nodes();				
				Poly::ComputeNormals(mesh); //we need the normals					

				if (hasSeam)
				{

					TColStd_Array1OfReal norms(1, mesh->Normals().Length());
					for (Standard_Integer i = 1; i <= mesh->NbNodes() * 3; i += 3) //visit each node
					{
						gp_Dir dir(mesh->Normals().Value(i), mesh->Normals().Value(i + 1), mesh->Normals().Value(i + 2));
						if (faceReversed) dir.Reverse();
						dir = quaternion.Multiply(dir);
						norms.SetValue(i, dir.X());
						norms.SetValue(i + 1, dir.Y());
						norms.SetValue(i + 2, dir.Z());
					}
					Dictionary<XbimPoint3DWithTolerance^, int>^ uniquePointsOnFace = gcnew Dictionary<XbimPoint3DWithTolerance^, int>(mesh->NbNodes());
					for (Standard_Integer j = 1; j <= mesh->NbNodes(); j++) //visit each node for vertices
					{
						gp_Pnt p = nodes.Value(j);
						XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
						int nodeIndex;
						if (uniquePointsOnFace->TryGetValue(pt, nodeIndex)) //we have a duplicate point on face need to smooth the normal
						{
							//balance the two normals
							gp_Vec normalA(norms.Value(nodeIndex), norms.Value(nodeIndex) + 1, norms.Value(nodeIndex) + 2);
							gp_Vec normalB(norms.Value(j), norms.Value(j) + 1, norms.Value(j) + 2);
							gp_Vec normalBalanced = normalA + normalB;
							normalBalanced.Normalize();
							norms.SetValue(nodeIndex, normalBalanced.X());
							norms.SetValue(nodeIndex + 1, normalBalanced.Y());
							norms.SetValue(nodeIndex + 2, normalBalanced.Z());
							norms.SetValue(j, normalBalanced.X());
							norms.SetValue(j + 1, normalBalanced.Y());
							norms.SetValue(j + 2, normalBalanced.Z());
						}
						else
							uniquePointsOnFace->Add(pt, j);
					}
					//write the nodes
					for (Standard_Integer j = 0; j < mesh->NbNodes(); j++) //visit each node for vertices
					{
						gp_Pnt p = nodes.Value(j + 1);
						Standard_Real px = p.X();
						Standard_Real py = p.Y();
						Standard_Real pz = p.Z();
						transform.Transforms(px, py, pz); //transform the point to the right location
						gp_Dir dir(norms.Value((j * 3) + 1), norms.Value((j * 3) + 2), norms.Value((j * 3) + 3));
						meshReceiver->AddNode(faceId, px, py, pz, dir.X(), dir.Y(), dir.Z()); //add the node to the face
					}
				}
				else //write the nodes
				{
					for (Standard_Integer j = 0; j < mesh->NbNodes(); j++) //visit each node for vertices
					{
						gp_Pnt p = nodes.Value(j + 1);
						Standard_Real px = p.X();
						Standard_Real py = p.Y();
						Standard_Real pz = p.Z();
						transform.Transforms(px, py, pz); //transform the point to the right location
						gp_Dir dir(mesh->Normals().Value((j * 3) + 1), mesh->Normals().Value((j * 3) + 2), mesh->Normals().Value((j * 3) + 3));
						if (faceReversed) dir.Reverse();
						dir = quaternion.Multiply(dir); //rotate the norm to the new location
						meshReceiver->AddNode(faceId, px, py, pz, dir.X(), dir.Y(), dir.Z()); //add the node to the face
					}
				}

				Standard_Integer t[3];
				const Poly_Array1OfTriangle& triangles = mesh->Triangles();

				for (Standard_Integer j = 1; j <= mesh->NbTriangles(); j++) //add each triangle as a face
				{
					if (faceReversed) //get nodes in the correct order of triangulation
						triangles(j).Get(t[2], t[1], t[0]);
					else
						triangles(j).Get(t[0], t[1], t[2]);
					meshReceiver->AddTriangle(faceId, t[0] - 1, t[1] - 1, t[2] - 1);
				}
			}
			GC::KeepAlive(this);
			
		}



	


		void XbimOccShape::WriteIndex(BinaryWriter^ bw, UInt32 index, UInt32 maxInt)
		{
			if (maxInt <= 0xFF)
				bw->Write((unsigned char)index);
			else if (maxInt <= 0xFFFF)
				bw->Write((UInt16)index);
			else
				bw->Write(index);
		}

		void XbimOccShape::WriteTriangulation(BinaryWriter^ binaryWriter, double tolerance, double deflection, double angle)
		{

			if (!IsValid) return;
			TopTools_IndexedMapOfShape faceMap;
			TopoDS_Shape shape = this; //hold on to it
			TopExp::MapShapes(shape, TopAbs_FACE, faceMap);			
			int faceCount = faceMap.Extent();
			if (faceCount == 0) return;
			
			Dictionary<XbimPoint3DWithTolerance^, int>^ pointMap = gcnew Dictionary<XbimPoint3DWithTolerance^, int>();
			List<List<int>^>^ pointLookup = gcnew List<List<int>^>(faceCount);
			List<XbimPoint3D>^ points = gcnew List<XbimPoint3D>(faceCount * 3);;

			Dictionary<int, int>^ normalMap = gcnew Dictionary<int, int>();
			List<List<XbimPackedNormal>^>^ normalLookup = gcnew List<List<XbimPackedNormal>^>(faceCount);
			
			//First write out all the vertices
			int faceIndex = 0;
			int triangleCount = 0;
			List<List<int>^>^ tessellations = gcnew List<List<int>^>(faceCount);
			bool isPolyhedron = true;
			array<bool>^ hasSeams = gcnew array<bool>(faceCount);
			for (int f = 1; f <= faceMap.Extent(); f++)
			{
				TopTools_IndexedMapOfShape edgeMap;
				TopExp::MapShapes(faceMap(f), TopAbs_EDGE, edgeMap);
				hasSeams[f - 1] = false;
				for (Standard_Integer i = 1; i <= edgeMap.Extent(); i++)
				{
					Standard_Real start, end;
					//find any seams					
					if (!hasSeams[f - 1]) hasSeams[f - 1] = (BRep_Tool::IsClosed(edgeMap(i))==Standard_True); //just check a seam once
					Handle(Geom_Curve) c3d = BRep_Tool::Curve(TopoDS::Edge(edgeMap(i)), start, end);
					if (!c3d.IsNull())
					{
						Handle(Standard_Type) cType = c3d->DynamicType();
						if (cType != STANDARD_TYPE(Geom_Line))
						{
							if (cType != STANDARD_TYPE(Geom_TrimmedCurve)) 
							{
								isPolyhedron = false; 
							//	isCurveFace[f - 1] = true;
								continue;
							}
							Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c3d);
							Handle(Standard_Type) tcType = tc->DynamicType();
							if (tcType != STANDARD_TYPE(Geom_Line)) 
							{
								isPolyhedron = false;
								//isCurveFace[f - 1] = true;
								continue;
							}
						}
					}				
				}			
			}
			if (!isPolyhedron)				
				BRepMesh_IncrementalMesh incrementalMesh(this, deflection, Standard_False, angle); //triangulate the first time							
			for (int f = 1; f <= faceMap.Extent(); f++)
			{
				const TopoDS_Face& face = TopoDS::Face(faceMap(f));
				bool faceReversed = (face.Orientation() == TopAbs_REVERSED);
				//bool isFaceWithCurve = isCurveFace[f - 1];
				List<XbimPackedNormal>^ norms;
				Tess^ tess = gcnew Tess();
				if (!isPolyhedron)
				{								
					TopLoc_Location loc;
					const Handle(Poly_Triangulation)& mesh = BRep_Tool::Triangulation(face, loc);
					if (mesh.IsNull())
						continue;
					//check if we have a seam
					bool hasSeam = hasSeams[f - 1];
					gp_Trsf transform = loc.Transformation();
					gp_Quaternion quaternion =  transform.GetRotation();
					const TColgp_Array1OfPnt & nodes = mesh->Nodes();
					triangleCount += mesh->NbTriangles();
					pointLookup->Add(gcnew List<int>(mesh->NbNodes()));
					Poly::ComputeNormals(mesh); //we need the normals
					norms = gcnew List<XbimPackedNormal>(mesh->NbNodes());
					for (Standard_Integer i = 1; i <= mesh->NbNodes() * 3; i += 3) //visit each node
					{
						gp_Dir dir(mesh->Normals().Value(i), mesh->Normals().Value(i + 1), mesh->Normals().Value(i + 2));					
						if (faceReversed) dir.Reverse();
						
						dir = quaternion.Multiply(dir);
						XbimPackedNormal packedNormal = XbimPackedNormal(dir.X(), dir.Y(), dir.Z()); 						
						norms->Add(packedNormal);
					}
					normalLookup->Add(norms);
					Dictionary<XbimPoint3DWithTolerance^, int>^ uniquePointsOnFace = nullptr;
					for (Standard_Integer j = 1; j <= mesh->NbNodes(); j++) //visit each node for vertices
					{
						gp_XYZ p = nodes.Value(j).XYZ();
						transform.Transforms(p);
						int index;
						XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X(), p.Y(), p.Z(), tolerance);
						if (!pointMap->TryGetValue(pt, index))
						{
							index = points->Count;
							pointMap->Add(pt, index);
							points->Add(pt->VertexGeometry);
						}
						pointLookup[faceIndex]->Add(index);
						if (hasSeam) //keep a record of duplicate points on face triangulation so we can average the normals
						{
							if (uniquePointsOnFace == nullptr) uniquePointsOnFace = gcnew Dictionary<XbimPoint3DWithTolerance^, int>(mesh->NbNodes());
							int nodeIndex;
							if (uniquePointsOnFace->TryGetValue(pt, nodeIndex)) //we have a duplicate point on face need to smooth the normal
							{
								//balance the two normals
								XbimPackedNormal normalA = norms[nodeIndex-1];
								XbimPackedNormal normalB = norms[j-1];
								XbimVector3D vec = normalA.Normal + normalB.Normal;
								vec = vec.Normalized();
								XbimPackedNormal normalBalanced = XbimPackedNormal(vec);
								norms[nodeIndex-1] = normalBalanced;
								norms[j-1] = normalBalanced;
							}
							else
								uniquePointsOnFace->Add(pt, j);							
						}
					}
					Standard_Integer t[3];
					const Poly_Array1OfTriangle& triangles = mesh->Triangles();

					List<int>^ elems = gcnew List<int>(mesh->NbTriangles() * 3);
					for (Standard_Integer j = 1; j <= mesh->NbTriangles(); j++) //add each triangle as a face
					{
						if (faceReversed) //get nodes in the correct order of triangulation
							triangles(j).Get(t[2], t[1], t[0]);
						else
							triangles(j).Get(t[0], t[1], t[2]);
						elems->Add(t[0] - 1);
						elems->Add(t[1] - 1);
						elems->Add(t[2] - 1);
					}
					tessellations->Add(elems);
					faceIndex++;
					
				}
				else //it is planar we can use LibMeshDotNet
				{
					norms = gcnew List<XbimPackedNormal>(1);					
					//IXbimWireSet^ bounds = gcnew XbimWireSet(face);
					TopTools_IndexedMapOfShape wireMap;
					TopExp::MapShapes(face, TopAbs_WIRE, wireMap);
					List<array<ContourVertex>^>^ contours = gcnew List<array<ContourVertex>^>(wireMap.Extent());
					for (int i = 1; i <= wireMap.Extent(); i++)
					{
						TopoDS_Wire ccWire = TopoDS::Wire(wireMap(i));
						int t = 0;
						BRepTools_WireExplorer exp(ccWire);
						for (; exp.More(); exp.Next()) t++;
						if (t <= 2) continue;

						array<ContourVertex>^ contour = gcnew array<ContourVertex>(t);
						int j = 0;
						for (exp.Init(ccWire); exp.More(); exp.Next())
						{
							gp_Pnt p = BRep_Tool::Pnt(exp.CurrentVertex());
							contour[j].Position.X = (float)p.X();
							contour[j].Position.Y = (float)p.Y();
							contour[j].Position.Z = (float)p.Z();
							j++;
						}
						if (contour->Length > 0)
							contours->Add(contour);
					}

					if (contours->Count > 0)
					{
						tess->AddContours(contours, true);
						tess->Tessellate(Xbim::Tessellator::WindingRule::EvenOdd, Xbim::Tessellator::ElementType::Polygons, 3);
						int numTriangles = tess->ElementCount;
						triangleCount += numTriangles;

						array<ContourVertex>^ contourVerts = tess->Vertices;
						array<int>^ elements = tess->Elements;
						if (numTriangles > 0)
						{
							pointLookup->Add(gcnew List<int>(tess->VertexCount));
							XbimVector3D fn(tess->Normal[0], tess->Normal[1], tess->Normal[2]);
							XbimPackedNormal packedNormal = XbimPackedNormal(fn);							
							norms->Add(packedNormal);
							normalLookup->Add(norms);
							for (int i = 0; i < tess->VertexCount; i++) //visit each node for vertices
							{
								Vec3 p = contourVerts[i].Position;
								int index;
								XbimPoint3DWithTolerance^ pt = gcnew XbimPoint3DWithTolerance(p.X, p.Y, p.Z, tolerance);
								if (!pointMap->TryGetValue(pt, index))
								{
									index = points->Count;
									pointMap->Add(pt, index);
									points->Add(pt->VertexGeometry);
								}
								pointLookup[faceIndex]->Add(index);
							}
							List<int>^ elems = gcnew List<int>(numTriangles * 3);
							for (int j = 0; j < numTriangles; j++)
							{
								elems->Add(elements[j * 3]);
								elems->Add(elements[j * 3 + 1]);
								elems->Add(elements[j * 3 + 2]);
							}
							tessellations->Add(elems);
							faceIndex++;
						}
					}
				}
			}
			// Write out header
			binaryWriter->Write((unsigned char)1); //stream format version
			int numVertices = points->Count;
			binaryWriter->Write((UInt32)numVertices); //number of vertices
			binaryWriter->Write((UInt32)triangleCount); //number of triangles
			//write out vertices 
			for each (XbimPoint3D p in points)
			{
				binaryWriter->Write((float)p.X);
				binaryWriter->Write((float)p.Y);
				binaryWriter->Write((float)p.Z);
			}

			//now write out the faces
			faceIndex = 0;
			binaryWriter->Write((Int32)tessellations->Count);
			for each (List<int>^ tess in tessellations)
			{
				List<XbimPackedNormal>^ norms = normalLookup[faceIndex];
				bool isPlanar = norms->Count == 1;
				List<int>^ nodeLookup = pointLookup[faceIndex];
				if (isPlanar)
				{
					binaryWriter->Write((Int32)tess->Count / 3);
					norms[0].Write(binaryWriter); //write the normal for the face
				}
				else
					binaryWriter->Write((Int32)(-tess->Count / 3)); //use negative count to indicate that every index has a normal			
				for (int i = 0; i < tess->Count; i++)
				{
					if (isPlanar)
					{
						WriteIndex(binaryWriter, nodeLookup[tess[i]], numVertices);	
					}
					else //need to write every one
					{
						WriteIndex(binaryWriter, nodeLookup[tess[i]], numVertices);
						norms[tess[i]].Write(binaryWriter);
					}
				}	
				faceIndex++;
			}
			GC::KeepAlive(this);
			binaryWriter->Flush();
		}

		
	}
}
