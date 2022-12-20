#pragma once

#include <gp_XY.hxx>
#include <Precision.hxx>
#include <BRepBuilderAPI_VertexInspector.hxx>
#include "PointInspector.h"
#include <TColStd_SequenceOfInteger.hxx>
#include "Graphic3d_BndBox3d.h"
#include <Graphic3d_Vec3.hxx>
#include <TopoDS_Shape.hxx>
#include "NFaceMeshIterator.h"

typedef struct PackedNormal {
	unsigned char byte[2];
	PackedNormal(unsigned char u, unsigned char v) : byte{ u,v } { }
	unsigned char u() { return byte[0]; }
	unsigned char v() { return byte[1]; }
} PackedNormal;
typedef NCollection_Vector<int> VectorOfInt;
typedef NCollection_Vec3<int> Vec3OfInt;
typedef std::vector<PackedNormal> VectorOfPackedNormal;
typedef NCollection_Vector<Vec3OfInt> TriangleIndices;

typedef NCollection_Vector<TriangleIndices> VectorOfTriangleIndices;
typedef std::vector<VectorOfPackedNormal> VectorOfTriangleNormals;




class NWexBimMesh
{
private:
	double myTolerance;
	PointInspector pointInspector;
	VectorOfInt vertexIndexlookup;
	NCollection_CellFilter<PointInspector> pointFilter;
	VectorOfTriangleIndices indicesPerFace;
	VectorOfTriangleNormals normalsPerFace;
	
	
public:
	const unsigned char Version = 1;
	bool HasCurves = false;
	static NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale, bool checkEdges, bool cleanBefore, bool cleanAfter);
	static NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale);
	
	NWexBimMesh::NWexBimMesh(double tolerance) :myTolerance(tolerance), pointInspector(tolerance), pointFilter(tolerance), ByteOffet(0) {};
	
	void WriteTriangleIndicesWithNormals(
		std::ostream& oStream, 
		const NCollection_Vec3<int>& triangle,
		const PackedNormal& a,
		const PackedNormal& b,
		const PackedNormal& c, 
		unsigned int maxVertices);

	void WriteTriangleIndices(std::ostream& oStream, NCollection_Vec3<int> triangle, unsigned int numTriangles);
	
	int AddPoint(gp_XYZ point);
	int VertexCount() { return pointInspector.myPoints.Length(); }
	const VectorOfXYZ& Vertices() { return pointInspector.myPoints; }
	const gp_XYZ& LookupVertex(int idx) { return pointInspector.myPoints(vertexIndexlookup.Value(idx)); }
	void AddTriangleIndices(TriangleIndices triangleIndices) { indicesPerFace.Append(triangleIndices); }
	void AddNormals(VectorOfPackedNormal normals) { normalsPerFace.push_back(normals); };
	int FaceCount() { return indicesPerFace.Length(); }
	int TriangleCount();
	static PackedNormal ToPackedNormal(const Graphic3d_Vec3& vec3);
	const VectorOfTriangleNormals& NormalsPerFace() {return normalsPerFace;}
	std::streampos ByteOffet;
	void WriteToStream(std::ostream & oStream);
	
	Graphic3d_BndBox3d          BndBox;        //!< bounding box
	
private:
	void saveIndices(const NFaceMeshIterator& theFaceIter) ;
	void saveNormals(NFaceMeshIterator& theFaceIter);
	void saveNodes(const NFaceMeshIterator& theFaceIter, double scale);
	
};
