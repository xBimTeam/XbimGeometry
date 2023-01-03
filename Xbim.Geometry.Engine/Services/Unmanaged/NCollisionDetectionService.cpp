#include "NCollisionDetectionService.h" 
#include <TopoDS_Shape.hxx> 
#include "NLoggingService.h" 
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <BRep_Builder.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepExtrema_ShapeProximity.hxx>
#include <BOPAlgo_BOP.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <IntTools_FaceFace.hxx>



bool NCollisionDetectionService::IsColliding(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection)
{
	BRepMesh_IncrementalMesh aMesh1(shape1, linearDeflection, Standard_False, angularDeflection);
	BRepMesh_IncrementalMesh aMesh2(shape2, linearDeflection, Standard_False, angularDeflection);
	    
	BRepExtrema_ShapeProximity proximity(shape1, shape2, precision);
	proximity.Perform();
	    
	return proximity.IsDone() && proximity.OverlapSubShapes1().Size() > 0;
}

