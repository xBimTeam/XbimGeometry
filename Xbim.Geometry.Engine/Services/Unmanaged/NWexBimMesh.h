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
#include "PackedNormal.h"


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
	double myScale;
	PointInspector pointInspector;
	
	NCollection_CellFilter<PointInspector> pointFilter;
	VectorOfTriangleIndices indicesPerFace;
	VectorOfTriangleNormals normalsPerFace;


public:
	const unsigned char Version = 1;
	bool HasCurves = false;
	static NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale, bool checkEdges, bool cleanBefore, bool cleanAfter);
	static NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale);

	NWexBimMesh::NWexBimMesh(double tolerance, double scale) :myTolerance(tolerance), myScale(scale), pointInspector(tolerance* scale), pointFilter(tolerance* scale), ByteOffet(0) {};

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
	
	void AddTriangleIndices(TriangleIndices triangleIndices) { indicesPerFace.Append(triangleIndices); }
	void AddNormals(VectorOfPackedNormal normals) { normalsPerFace.push_back(normals); };
	int FaceCount() { return indicesPerFace.Length(); }
	int TriangleCount();
	
	const VectorOfTriangleNormals& NormalsPerFace() { return normalsPerFace; }
	std::streampos ByteOffet;
	void WriteToStream(std::ostream& oStream);

	Graphic3d_BndBox3d          BndBox;        //!< bounding box

private:
	void saveIndicesAndNormals(NFaceMeshIterator& theFaceIter);

	void saveNodes(const NFaceMeshIterator& theFaceIter, std::vector<int>& nodeIndexes);

};
