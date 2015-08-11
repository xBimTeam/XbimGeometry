#include "XbimOccShape.h"
#include "XbimFaceSet.h"
#include "XbimPoint3DWithTolerance.h"
#include "XbimGeomPrim.h"
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
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace Xbim::Tessellator;

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
				const Handle_Poly_Triangulation& mesh = BRep_Tool::Triangulation(face, loc);
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
				const Handle_Poly_Triangulation& mesh = BRep_Tool::Triangulation(face, loc);
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
				textWriter->Flush();
				GC::KeepAlive(this);
			}
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

			XbimFaceSet^ faces = gcnew XbimFaceSet(this);

			if (faces->Count == 0) return;

			Dictionary<XbimPoint3DWithTolerance^, int>^ pointMap = gcnew Dictionary<XbimPoint3DWithTolerance^, int>();
			List<List<int>^>^ pointLookup = gcnew List<List<int>^>(faces->Count);
			List<XbimPoint3D>^ points = gcnew List<XbimPoint3D>(faces->Count * 3);;

			Dictionary<int, int>^ normalMap = gcnew Dictionary<int, int>();
			List<List<int>^>^ normalLookup = gcnew List<List<int>^>(faces->Count);
			List<XbimPackedNormal>^ normals = gcnew List<XbimPackedNormal>(faces->Count);
			//List<XbimFace^>^ writtenFaces = gcnew List<XbimFace^>(faces->Count);
			//First write out all the vertices
			int faceIndex = 0;
			int triangleCount = 0;
			List<List<int>^>^ tessellations = gcnew List<List<int>^>(faces->Count);
			
			for each (XbimFace^ face in faces)
			{
				bool faceReversed = face->IsReversed;
				bool isPolygonal = face->IsPolygonal;
				List<int>^ norms;
				Tess^ tess = gcnew Tess();
				if (!isPolygonal)
				{
					Bnd_Box pBox;
					BRepBndLib::Add(face, pBox);
					BRepMesh_FastDiscret faceMesher(face, deflection, angle, pBox, Standard_True);
					TopLoc_Location loc;
					const Handle_Poly_Triangulation& mesh = BRep_Tool::Triangulation(face, loc);
					if (mesh.IsNull())
						continue;
					gp_Trsf transform = loc.Transformation();
					gp_Quaternion quaternion =  transform.GetRotation();
					const TColgp_Array1OfPnt & nodes = mesh->Nodes();
					triangleCount += mesh->NbTriangles();
					pointLookup->Add(gcnew List<int>(mesh->NbNodes()));
					Poly::ComputeNormals(mesh); //we need the normals
					norms = gcnew List<int>(mesh->NbNodes());
					for (Standard_Integer i = 1; i <= mesh->NbNodes() * 3; i += 3) //visit each node
					{
						gp_Dir dir(mesh->Normals().Value(i), mesh->Normals().Value(i + 1), mesh->Normals().Value(i + 2));					
						if (faceReversed) dir.Reverse();
						int index;
						dir = quaternion.Multiply(dir);
						XbimPackedNormal packedNormal = XbimPackedNormal(dir.X(), dir.Y(), dir.Z()); 
						int packedVal = packedNormal.ToUnit16();
						if (!normalMap->TryGetValue(packedVal, index))
						{
							index = normalMap->Count;
							normalMap->Add(packedVal, index);
							normals->Add(packedNormal);
						}
						norms->Add(index);
					}
					normalLookup->Add(norms);
					for (Standard_Integer i = 1; i <= mesh->NbNodes(); i++) //visit each node for vertices
					{
						gp_XYZ p = nodes.Value(i).XYZ();
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
					}
					Standard_Integer t[3];
					const Poly_Array1OfTriangle& triangles = mesh->Triangles();

					List<int>^ elems = gcnew List<int>(mesh->NbTriangles() * 3);
					for (Standard_Integer i = 1; i <= mesh->NbTriangles(); i++) //add each triangle as a face
					{
						if (faceReversed) //get nodes in the correct order of triangulation
							triangles(i).Get(t[2], t[1], t[0]);
						else
							triangles(i).Get(t[0], t[1], t[2]);
						elems->Add(t[0] - 1);
						elems->Add(t[1] - 1);
						elems->Add(t[2] - 1);
					}
					tessellations->Add(elems);
					faceIndex++;
				}
				else //it is planar we can use LibMeshDotNet
				{
					norms = gcnew List<int>(1);					
					IXbimWireSet^ bounds = face->Bounds;
					List<array<ContourVertex>^>^ contours = gcnew List<array<ContourVertex>^>(bounds->Count);
					for each (XbimWire^ bound in bounds)
					{						
						array<ContourVertex>^ contour = bound->Contour();
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
							int index;
							XbimVector3D fn(tess->Normal[0], tess->Normal[1], tess->Normal[2]);
							XbimPackedNormal packedNormal = XbimPackedNormal(fn);
							int packedVal = packedNormal.U << 8 | packedNormal.V;

							if (!normalMap->TryGetValue(packedVal, index))
							{
								index = normalMap->Count;
								normalMap->Add(packedVal, index);
								normals->Add(packedNormal);
							}
							norms->Add(index);
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
				List<int>^ norms = normalLookup[faceIndex];
				bool isPlanar = norms->Count == 1;
				List<int>^ nodeLookup = pointLookup[faceIndex];
				if (isPlanar)
				{
					binaryWriter->Write((Int32)tess->Count / 3);
					normals[norms[0]].Write(binaryWriter); //write the normal for the face
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
						normals[norms[tess[i]]].Write(binaryWriter);
					}
				}	
				faceIndex++;
			}
			GC::KeepAlive(this);
			binaryWriter->Flush();
		}
		
	}
}
