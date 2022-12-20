#include "NShapeService.h"

#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Plane.hxx>
#include <TopExp_Explorer.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Quaternion.hxx>
#include <Poly.hxx>
#include <gp_Dir.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopoDS.hxx>



//bool NShapeService::AnalyseShape(const TopTools_IndexedMapOfShape& faceMap, bool& isPolyhedron, std::vector<bool>& seams)
//{
//	try
//	{
//		isPolyhedron = true;
//		for (int f = 1; f <= faceMap.Extent(); f++)
//		{
//			const TopoDS_Face& face = TopoDS::Face(faceMap(f));
//			Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(BRep_Tool::Surface(face));
//			bool isPlane = !plane.IsNull();
//			//set the seam value to false for default, seams cannot be on planar surfaces
//			seams.push_back(false);
//			if (!isPlane) isPolyhedron = false; //must be a plane to be a polyhedron
//			if (isPolyhedron && isPlane) //if the shape is still potentially a polyhedron then check that this planar face has no curves
//			{
//				for (TopExp_Explorer edgeExplorer(face, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next())
//				{
//					Standard_Real start, end;
//					Handle(Geom_Curve) c3d = BRep_Tool::Curve(TopoDS::Edge(edgeExplorer.Current()), start, end);
//					if (!c3d.IsNull())
//					{
//						if (c3d->DynamicType() == STANDARD_TYPE(Geom_Line)) //if it is a line all is well skip to next edge
//							continue;
//
//						if (c3d->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve)) //if it is a trimmed curve determine if basis curve is a line
//						{
//							//it must be a trimmed curve
//							Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c3d);
//							//flatten any trimeed curve nesting
//							while (tc->BasisCurve()->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
//								tc = Handle(Geom_TrimmedCurve)::DownCast(tc->BasisCurve());
//							//get the type of the basis curve
//							Handle(Standard_Type) tcType = tc->BasisCurve()->DynamicType();
//							//if its a line all is well skip to next edge
//							if (tcType == STANDARD_TYPE(Geom_Line))
//								continue;
//						}
//						//if here then the shape has curves and we need to use OCC meshing
//						isPolyhedron = false;
//						break;
//					}
//				}
//			}
//			if (!isPlane) //curved surface check for any seams that will need smoothing
//			{
//				for (TopExp_Explorer edgeExplorer(face, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next())
//				{
//					//find any seams					
//					bool isSeam = (BRep_Tool::IsClosed(edgeExplorer.Current()) == Standard_True);
//					if (isSeam)
//					{
//						seams.at(f - 1) = true; //just check a seam once	
//						break;
//					}
//				}
//			}
//		}
//		return true;
//	}
//	catch (const Standard_Failure&)
//	{
//		return false;
//	}
//}
//
//int NShapeService::TriangulateNonPolygonalFace(const TopoDS_Shape& shape, double oneMeter, double tolerance, double linearDeflection, double angularDeflection)
//{
//	TopTools_IndexedMapOfShape faceMap;
//	TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
//	
//	BRepMesh_IncrementalMesh incrementalMesh(shape, linearDeflection, Standard_False, angularDeflection); //triangulate the first time	
//	int triangleCount = 0;
//	std::vector<std::vector<int>> tessellations;
//	std::vector<std::vector<int>> pointLookup;
//	BRepBuilderAPI_VertexInspector inspector(tolerance);
//	NCollection_CellFilter<BRepBuilderAPI_VertexInspector> pointCells(2, oneMeter/1000);
//	std::vector<gp_XYZ> uniquePoints;
//	std::vector<std::vector<gp_Dir>> normalLookup;
//	for (size_t i = 1; i < faceMap.Extent(); i++)
//	{
//		int triangleCount = 0;
//		const TopoDS_Face& face = TopoDS::Face(faceMap(i));
//		if (face.IsNull()) continue;
//		bool faceReversed = (face.Orientation() == TopAbs_REVERSED);
//		Handle(Geom_Plane) plane = Handle(Geom_Plane)::DownCast(BRep_Tool::Surface(face));
//		bool isPlanar = !plane.IsNull();
//		TopLoc_Location loc;
//		const Handle(Poly_Triangulation)& mesh = BRep_Tool::Triangulation(face, loc);
//		if (mesh.IsNull())
//			return false;
//		triangleCount += mesh->NbTriangles();
//		gp_Trsf transform = loc.Transformation();
//		gp_Quaternion quaternion = transform.GetRotation();
//
//		if (!isPlanar)
//		{
//			Poly::ComputeNormals(mesh); //we need the normals
//			std::vector<gp_Dir> norms;
//			norms.reserve(mesh->NbNodes());
//			for (Standard_Integer i = 1; i <= mesh->NbNodes(); i++) //visit each node
//			{
//				gp_Dir dir = mesh->Normal(i);
//				if (faceReversed) dir.Reverse();
//				dir = quaternion.Multiply(dir);
//				norms.push_back(dir);
//			}
//			normalLookup.push_back(norms);
//		}
//		else //just need one normal
//		{
//			std::vector<gp_Dir> norms;
//			gp_Dir faceNormal = faceReversed ? plane->Axis().Direction().Reversed() : plane->Axis().Direction();
//			norms.push_back(faceNormal);
//			normalLookup.push_back(norms);
//		}
//
//
//		std::vector<int> pointIds;
//		pointIds.reserve(mesh->NbNodes());
//
//		for (Standard_Integer j = 1; j <= mesh->NbNodes(); j++) //visit each node for vertices
//		{
//			gp_XYZ coord = mesh->Node(j).XYZ();
//			transform.Transforms(coord);
//
//			inspector.ClearResList();
//			inspector.SetCurrent(coord);
//			pointCells.Inspect(coord, inspector);
//			const TColStd_ListOfInteger& results = inspector.ResInd();
//
//			if (results.Size() > 0) //hit
//			{
//				//just take the first one as we don't add vertices more than once to a cell
//				int pointId = results.First();
//				pointIds.push_back(pointId);
//			}
//			else //miss
//			{
//				inspector.Add(coord);
//				uniquePoints.push_back(coord); //it will have the same index as the point in the inspector
//				pointCells.Add(uniquePoints.size(), coord);
//				pointIds.push_back(uniquePoints.size() - 1);
//			}
//
//			//if (hasSeam) //keep a record of duplicate points on face triangulation so we can average the normals
//			//{		
//			//	int nodeIndex;
//			//	if (uniquePointsOnFace->TryGetValue(pt, nodeIndex)) //we have a duplicate point on face need to smooth the normal
//			//	{
//			//		//balance the two normals
//			//		XbimPackedNormal normalA = norms[nodeIndex - 1];
//			//		XbimPackedNormal normalB = norms[j - 1];
//			//		XbimVector3D vec = normalA.Normal + normalB.Normal;
//			//		vec = vec.Normalized();
//			//		XbimPackedNormal normalBalanced = XbimPackedNormal(vec);
//			//		norms[nodeIndex - 1] = normalBalanced;
//			//		norms[j - 1] = normalBalanced;
//			//	}
//			//	else
//			//		uniquePointsOnFace->Add(pt, j);
//			//}
//		}
//		pointLookup.push_back(pointIds);
//		Standard_Integer t[3];
//
//		std::vector<int> elems;
//		elems.reserve(mesh->NbTriangles() * 3);
//		for (Standard_Integer j = 1; j <= mesh->NbTriangles(); j++) //add each triangle as a face
//		{
//			if (faceReversed) //get nodes in the correct order of triangulation
//				mesh->Triangle(j).Get(t[2], t[1], t[0]);
//			else
//				mesh->Triangle(j).Get(t[0], t[1], t[2]);
//			elems.push_back(t[0] - 1);
//			elems.push_back(t[1] - 1);
//			elems.push_back(t[2] - 1);
//		}
//		tessellations.push_back(elems);
//	}
//
//	// Write out header
//	binaryWriter->Write((unsigned char)1); //stream format version
//	int numVertices = points->Count;
//	binaryWriter->Write((System::UInt32)numVertices); //number of vertices
//	binaryWriter->Write((System::UInt32)triangleCount); //number of triangles
//	//write out vertices 
//	for each (XbimPoint3D p in points)
//	{
//		binaryWriter->Write((float)p.X);
//		binaryWriter->Write((float)p.Y);
//		binaryWriter->Write((float)p.Z);
//	}
//
//	//now write out the faces
//	faceIndex = 0;
//	binaryWriter->Write((System::Int32)tessellations->Count);
//	for each (List<int> ^ tess in tessellations)
//	{
//		List<XbimPackedNormal>^ norms = normalLookup[faceIndex];
//		bool isPlanar = norms->Count == 1;
//		List<int>^ nodeLookup = pointLookup[faceIndex];
//		if (isPlanar)
//		{
//			binaryWriter->Write((System::Int32)tess->Count / 3);
//			norms[0].Write(binaryWriter); //write the normal for the face
//		}
//		else
//			binaryWriter->Write((System::Int32)(-tess->Count / 3)); //use negative count to indicate that every index has a normal			
//		for (int i = 0; i < tess->Count; i++)
//		{
//			if (isPlanar)
//			{
//				WriteIndex(binaryWriter, nodeLookup[tess[i]], numVertices);
//			}
//			else //need to write every one
//			{
//				WriteIndex(binaryWriter, nodeLookup[tess[i]], numVertices);
//				norms[tess[i]].Write(binaryWriter);
//			}
//		}
//		faceIndex++;
//	}
//
//	binaryWriter->Flush();
//	
//
//}


