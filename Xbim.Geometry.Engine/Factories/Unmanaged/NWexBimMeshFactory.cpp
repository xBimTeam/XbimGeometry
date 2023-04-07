#include "NWexBimMeshFactory.h"
#include <BRep_Tool.hxx>
#include <Geom_Plane.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly.hxx>

 
NWexBimMesh NWexBimMeshFactory::CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale)
{
	return CreateMesh(shape, tolerance, linearDeflection, angularDeflection, scale, false, true, true);
}

NWexBimMesh NWexBimMeshFactory::CreateMesh(const TopoDS_Shape& shape,
											double tolerance, 
											double linearDeflection, 
											double angularDeflection, 
											double scale, 
											bool checkEdges, 
											bool cleanBefore, 
											bool cleanAfter)
{
	NWexBimMesh mesh(tolerance, scale);
	try
	{
		if (cleanBefore) BRepTools::Clean(shape); //remove triangulation
		BRepMesh_IncrementalMesh incrementalMesh(shape, linearDeflection, Standard_False, angularDeflection); //triangulate the first time	

		for (NFaceMeshIterator aFaceIter(shape, checkEdges); aFaceIter.More(); aFaceIter.Next())
		{
			if (aFaceIter.IsEmptyMesh())
				continue;
			if (!mesh.HasCurves) mesh.HasCurves = aFaceIter.HasCurves;

			mesh.SaveIndicesAndNormals(aFaceIter);

		}
		if (cleanAfter) BRepTools::Clean(shape); //remove triangulation

	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e, "WexBim mesh creation error.");
	}
	return mesh;
}

