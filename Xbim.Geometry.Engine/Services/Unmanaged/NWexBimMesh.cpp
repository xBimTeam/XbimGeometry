#include "NWexBimMesh.h"

#include <BRep_Tool.hxx>
#include <Geom_Plane.hxx>
#include <BRepMesh_IncrementalMesh.hxx>


int NWexBimMesh::TriangleCount()
{
	int triangleCount = 0;
	for (auto& it = indicesPerFace.cbegin(); it != indicesPerFace.cend(); it++)
	{
		triangleCount += it->Length();
	}
	return triangleCount;
}

PackedNormal NWexBimMesh::ToPackedNormal(const gp_Dir& vec)
{
	const double PackSize = 252;
	const double PackTolerance = std::tan(1 / PackSize);
	static  PackedNormal SingularityNegativeY((unsigned char)PackSize, (unsigned char)PackSize);
	static  PackedNormal SingularityPositiveY(0, 0);
	static double HalfPI = M_PI / 2;
	static double PIplusHalfPI = M_PI + HalfPI;
	static double TwoPI = M_PI * 2;
	/*int length = vec.Length();
	if (std::Abs(length - 1.0) > 1e-5)
	{
		throw new Exception("Vector has to be normalized before it is packed.");
	}*/

	//the most basic case when normal points in Y direction (singular point)
	if (std::abs(1 - vec.Y()) < PackTolerance)
		return SingularityPositiveY;
	//the most basic case when normal points in -Y direction (second singular point)
	if (std::abs(vec.Y() + 1) < PackTolerance)
		return SingularityNegativeY;

	double lat; //effectively XY
	double lon; //effectively Z
	//special cases when vector is aligned to one of the axis
	if (std::abs(vec.Z() - 1) < PackTolerance)
	{
		lon = 0;
		lat = HalfPI;
	}
	else if (std::abs(vec.Z() + 1) < PackTolerance)
	{
		lon = M_PI;
		lat = HalfPI;
	}
	else if (std::abs(vec.X() - 1) < PackTolerance)
	{
		lon = HalfPI;
		lat = HalfPI;
	}
	else if (std::abs(vec.X() + 1) < PackTolerance)
	{
		lon = PIplusHalfPI;
		lat = HalfPI;
	}
	else
	{
		//Atan2 takes care for quadrants (goes from positive Z to positive X and around)
		lon = std::atan2(vec.X(), vec.Z());
		//latitude from 0 to PI starting at positive Y ending at negative Y
		lat = std::acos(vec.Y());
	}

	//normalize values
	lon = lon / TwoPI;
	lat = lat / M_PI;

	//stretch to pack size so that round directions are aligned to axes.
	PackedNormal result((unsigned char)(lon * PackSize), (unsigned char)(lat * PackSize));
	return result;
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

				//the normals are in the same order as the triangle indices
				const PackedNormal& a = normalIt[0];
				const PackedNormal& b = normalIt[1];
				const PackedNormal& c = normalIt[2];
				normalIt++;
				WriteTriangleIndicesWithNormals(oStream, *triangleIt, a, b, c, numVertices);
			}
		}
	}
}
NWexBimMesh NWexBimMesh::CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale)
{
	return CreateMesh(shape, tolerance, linearDeflection, angularDeflection, scale, false, true, true);
}

NWexBimMesh NWexBimMesh::CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale, bool checkEdges, bool cleanBefore, bool cleanAfter)
{
	NWexBimMesh mesh(tolerance,scale);
	try
	{
		if (cleanBefore) BRepTools::Clean(shape); //remove triangulation
		BRepMesh_IncrementalMesh incrementalMesh(shape, linearDeflection, Standard_False, angularDeflection); //triangulate the first time	

		for (NFaceMeshIterator aFaceIter(shape, checkEdges); aFaceIter.More(); aFaceIter.Next())
		{
			if (aFaceIter.IsEmptyMesh())
				continue;
			if (!mesh.HasCurves) mesh.HasCurves = aFaceIter.HasCurves;
			mesh.saveIndicesAndNormals(aFaceIter);

		}
		if (cleanAfter) BRepTools::Clean(shape); //remove triangulation

	}
	catch (const Standard_Failure&)
	{

	}
	return mesh;
}

// =======================================================================
// function : saveNodes
// purpose  :
// =======================================================================
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


// =======================================================================
// function : saveIndices
// purpose  :
// =======================================================================
void NWexBimMesh::saveIndicesAndNormals(NFaceMeshIterator& theFaceIter)
{
	const Standard_Integer anElemLower = theFaceIter.ElemLower();
	const Standard_Integer anElemUpper = theFaceIter.ElemUpper();
	TriangleIndices triangleIndices;
	bool donePlanar = false;
	Handle(Geom_Surface) surf = BRep_Tool::Surface(theFaceIter.Face()); //the surface
	Handle(Geom_Plane) hPlane = Handle(Geom_Plane)::DownCast(surf);
	bool isPlanar = !hPlane.IsNull();
	VectorOfPackedNormal normals;
	bool hasNormals = theFaceIter.HasNormals();

	//first save the nodes for this face and remvoe duplicates
	std::vector<int> nodeIndexes;
	saveNodes(theFaceIter, nodeIndexes);
	for (Standard_Integer anElemIter = anElemLower; anElemIter <= anElemUpper; ++anElemIter)
	{
		Poly_Triangle aTri = theFaceIter.TriangleOriented(anElemIter);
		Poly_Triangle aFaceTri;


		//lookup up the actual node index, in the unique node list
		aFaceTri(1) = nodeIndexes[aTri(1) - anElemLower ];
		aFaceTri(2) = nodeIndexes[aTri(2) - anElemLower ];
		aFaceTri(3) = nodeIndexes[aTri(3) - anElemLower ];
		triangleIndices.Append(Vec3OfInt(aFaceTri(1), aFaceTri(2), aFaceTri(3)));

		if (hasNormals)
		{
			if (donePlanar)
				continue;
			else
				if (isPlanar) donePlanar = true;
			gp_Dir aNormal = theFaceIter.NormalTransformed(aTri(1));
			normals.push_back(NWexBimMesh::ToPackedNormal(aNormal));
			if (!isPlanar)
			{
				aNormal = theFaceIter.NormalTransformed(aTri(2));
				normals.push_back(NWexBimMesh::ToPackedNormal(aNormal));
				aNormal = theFaceIter.NormalTransformed(aTri(3));
				normals.push_back(NWexBimMesh::ToPackedNormal(aNormal));
			}
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
	if (idx >= 0) //hit and existing one
	{
		//just take the first one as we don't add vertices more than once to a cell
		vertexIndexlookup.Append(idx);
		return idx;
	}
	else //else keep it and add
	{
		int newIdx = pointInspector.Add(point) - 1;
		/*gp_XYZ coordMin = pointInspector.Shift(point, -myTolerance);
		gp_XYZ coordMax = pointInspector.Shift(point, myTolerance);*/
		pointFilter.Add(newIdx, point);
		vertexIndexlookup.Append(newIdx);
		return newIdx;
	}


}

