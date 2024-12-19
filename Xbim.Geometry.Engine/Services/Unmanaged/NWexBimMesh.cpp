#include "NWexBimMesh.h"
#include <BRep_Tool.hxx>
#include <Geom_Plane.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly.hxx>


int NWexBimMesh::TriangleCount()
{
	int triangleCount = 0;
	for (auto& it = indicesPerFace.cbegin(); it != indicesPerFace.cend(); it++)
	{
		triangleCount += it->Length();
	}
	return triangleCount;
}

void NWexBimMesh::WriteToStream(std::ostream& oStream)
{
	ByteOffet = oStream.tellp(); //save where this is in the file

	unsigned int numVertices = VertexCount();
	unsigned int numTriangles = TriangleCount();
	oStream.write((const char*)&Version, sizeof(Version));//stream format version
	oStream.write((const char*)&numVertices, sizeof(numVertices)); //number of vertices
	oStream.write((const char*)&numTriangles, sizeof(numTriangles)); //number of triangles
	//write out vertices 
	for (auto&& vertex : Vertices())
	{
		Graphic3d_Vec3 aVec3(float(vertex.X()), float(vertex.Y()), float(vertex.Z()));
		oStream.write((const char*)aVec3.GetData(), sizeof(aVec3));
	}
	//write out the faces
	int faceIndex = 0;
	int faceCount = FaceCount();
	oStream.write((const char*)&faceCount, sizeof(faceCount)); //write face count
	auto& normVecIt = normalsPerFace.begin();
	auto& indicesIt = indicesPerFace.cbegin();
	for (; indicesIt != indicesPerFace.cend(); indicesIt++, normVecIt++, faceIndex++)
	{
		bool isPlanar = normVecIt->size() == 1;
		int numTrianglesForFace = indicesIt->Length();
		if (isPlanar)
		{
			PackedNormal packedNormal = normVecIt->front();
			oStream.write((const char*)&numTrianglesForFace, sizeof(numTrianglesForFace));//write the number of triangles for the face
			oStream.write((const char*)&packedNormal, sizeof(packedNormal)); //write the normal for the face	
		}
		else
		{
			numTrianglesForFace *= -1; //use negative count to indicate that every index has a normal			
			oStream.write((const char*)&numTrianglesForFace, sizeof(numTrianglesForFace));
		}

		auto& triangleIt = indicesIt->cbegin();
		int iLen = indicesIt->Length();
		int nLen = (int)normVecIt->size();
		auto& normalIt = normVecIt->cbegin();
		for (; triangleIt != indicesIt->cend(); triangleIt++)
		{
			if (isPlanar)
			{
				WriteTriangleIndices(oStream, *triangleIt, numVertices);
			}
			else //need to write every normal as well
			{
				const PackedNormal& a = *normalIt; normalIt++;
				const PackedNormal& b = *normalIt; normalIt++;
				const PackedNormal& c = *normalIt; normalIt++;
				WriteTriangleIndicesWithNormals(oStream, *triangleIt, a, b, c, numVertices);

			}
		}
	}
}

void NWexBimMesh::saveNodes(const NFaceMeshIterator& theFaceIter, std::vector<int>& nodeIndexes)
{
	const Standard_Integer aNodeUpper = theFaceIter.NodeUpper();
	for (Standard_Integer aNodeIter = theFaceIter.NodeLower(); aNodeIter <= aNodeUpper; ++aNodeIter)
	{
		gp_XYZ aNode = theFaceIter.NodeTransformed(aNodeIter).XYZ();
		aNode.Multiply(myScale);
		//myCSTrsf.TransformPosition(aNode);
		BndBox.Add(BVH_Vec3d(aNode.X(), aNode.Y(), aNode.Z()));
		nodeIndexes.push_back(AddPoint(aNode));
	}
}

void NWexBimMesh::SaveIndicesAndNormals(NFaceMeshIterator& theFaceIter)
{
	const Standard_Integer anElemLower = theFaceIter.ElemLower();
	const Standard_Integer anElemUpper = theFaceIter.ElemUpper();
	TriangleIndices triangleIndices;
	bool donePlanar = false;
	
	bool isPlanar = theFaceIter.IsPlanar();
	VectorOfPackedNormal normals;

	//first save the nodes for this face and remove duplicates
	std::vector<int> nodeIndexes;
	saveNodes(theFaceIter, nodeIndexes);

	for (Standard_Integer anElemIter = anElemLower; anElemIter <= anElemUpper; ++anElemIter)
	{
		Poly_Triangle aTri = theFaceIter.TriangleOriented(anElemIter);
		Poly_Triangle aFaceTri;
		//lookup up the actual node index, in the unique node list
		aFaceTri(1) = nodeIndexes[aTri(1) - anElemLower];
		aFaceTri(2) = nodeIndexes[aTri(2) - anElemLower];
		aFaceTri(3) = nodeIndexes[aTri(3) - anElemLower];
		triangleIndices.Append(Vec3OfInt(aFaceTri(1), aFaceTri(2), aFaceTri(3)));

		if (donePlanar)
			continue;
		else
			if (isPlanar) donePlanar = true;
		gp_Dir aNormal = theFaceIter.NormalTransformed(aTri(1));
		normals.push_back(PackedNormal::ToPackedNormal(aNormal));
		if (!isPlanar)
		{
			aNormal = theFaceIter.NormalTransformed(aTri(2));
			normals.push_back(PackedNormal::ToPackedNormal(aNormal));
			aNormal = theFaceIter.NormalTransformed(aTri(3));
			normals.push_back(PackedNormal::ToPackedNormal(aNormal));
		}

	}
	AddTriangleIndices(triangleIndices);
	AddNormals(normals);
}

void NWexBimMesh::WriteTriangleIndicesWithNormals(
	std::ostream& oStream,
	const NCollection_Vec3<int>& triangle,
	const PackedNormal& a,
	const PackedNormal& b,
	const PackedNormal& c,
	unsigned int maxVertices)
{

	if (maxVertices <= 0xFF)
	{
		unsigned char x = (unsigned char)triangle.x();
		unsigned char y = (unsigned char)triangle.y();
		unsigned char z = (unsigned char)triangle.z();

		oStream.write((const char*)&x, sizeof(x));
		oStream.write((const char*)&a, sizeof(a));
		oStream.write((const char*)&y, sizeof(y));
		oStream.write((const char*)&b, sizeof(b));
		oStream.write((const char*)&z, sizeof(z));
		oStream.write((const char*)&c, sizeof(c));
	}
	else if (maxVertices <= 0xFFFF)
	{
		unsigned short x = (unsigned short)triangle.x();
		unsigned short y = (unsigned short)triangle.y();
		unsigned short z = (unsigned short)triangle.z();

		oStream.write((const char*)&x, sizeof(x));
		oStream.write((const char*)&a, sizeof(a));
		oStream.write((const char*)&y, sizeof(y));
		oStream.write((const char*)&b, sizeof(b));
		oStream.write((const char*)&z, sizeof(z));
		oStream.write((const char*)&c, sizeof(c));
	}
	else
	{
		int x = triangle.x();
		int y = triangle.y();
		int z = triangle.z();
		oStream.write((const char*)&x, sizeof(x));
		oStream.write((const char*)&a, sizeof(a));
		oStream.write((const char*)&y, sizeof(y));
		oStream.write((const char*)&b, sizeof(b));
		oStream.write((const char*)&z, sizeof(z));
		oStream.write((const char*)&c, sizeof(c));
	}
}

void NWexBimMesh::WriteTriangleIndices(std::ostream& oStream, NCollection_Vec3<int> triangle, unsigned int maxVertices)
{
	if (maxVertices <= 0xFF)
	{
		NCollection_Vec3<unsigned char> indices((unsigned char)triangle.x(), (unsigned char)triangle.y(), (unsigned char)triangle.z());
		oStream.write((const char*)indices.GetData(), sizeof(indices));
	}
	else if (maxVertices <= 0xFFFF)
	{
		NCollection_Vec3<uint16_t> indices((uint16_t)triangle.x(), (uint16_t)triangle.y(), (uint16_t)triangle.z());
		oStream.write((const char*)indices.GetData(), sizeof(indices));
	}
	else
		oStream.write((const char*)triangle.GetData(), sizeof(triangle));
}

int NWexBimMesh::AddPoint(gp_XYZ point)
{
	pointInspector.SetCurrent(point);
	pointFilter.Inspect(point, pointInspector);
	int idx = pointInspector.ResInd();
	if (idx >= 0) //hit an existing one
		return idx;
	else //else keep it and add
	{
		int newIdx = pointInspector.Add(point);
		pointFilter.Add(newIdx, point);
		return newIdx;
	}


}

