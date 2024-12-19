#include "NShapeProximityUtils.h" 
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
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BOPAlgo_BOP.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include "NProgressMonitor.h"

int NShapeProximityUtils::GetOverlappingSubShapes1Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection)
{
	BRepMesh_IncrementalMesh aMesh1(shape1, linearDeflection, Standard_False, angularDeflection);
	BRepMesh_IncrementalMesh aMesh2(shape2, linearDeflection, Standard_False, angularDeflection);
	    
	BRepExtrema_ShapeProximity proximity(shape1, shape2, precision);
	proximity.Perform();
	    
	if (proximity.IsDone())
		return proximity.OverlapSubShapes1().Size();
	
	return 0;
}

int NShapeProximityUtils::GetOverlappingSubShapes2Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection)
{
	BRepMesh_IncrementalMesh aMesh1(shape1, linearDeflection, Standard_False, angularDeflection);
	BRepMesh_IncrementalMesh aMesh2(shape2, linearDeflection, Standard_False, angularDeflection);

	BRepExtrema_ShapeProximity proximity(shape1, shape2, precision);
	proximity.Perform();

	if (proximity.IsDone())
		return proximity.OverlapSubShapes2().Size();

	return 0;
}

bool NShapeProximityUtils::IsOverlapping(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double tolerance, double linearDeflection, double angularDeflection)
{
	BRepMesh_IncrementalMesh aMesh1(shape1, linearDeflection, Standard_False, angularDeflection);
	BRepMesh_IncrementalMesh aMesh2(shape2, linearDeflection, Standard_False, angularDeflection);

	BRepExtrema_ShapeProximity proximity(shape1, shape2, tolerance);
	proximity.Perform();
	 
	if (proximity.IsDone()) {

		bool overlapping = proximity.OverlapSubShapes1().Size() > 0 && 
				proximity.OverlapSubShapes2().Size() > 0;

		//TODO (Ibrahim): better tangency check?
		//  If tolerance is set to zero, the algorithm will detect only intersecting faces
		//  (containing triangles with common points), faces in same plane is considered intersecting
		//  we need to discard result s with no actual geometry overlap
		if (overlapping && tolerance == 0) {
				return NShapeProximityUtils::HasOverlappingGeometry(shape1, shape2, tolerance);
		}

		return overlapping;
	}

	return false;
}

bool NShapeProximityUtils::HasOverlappingGeometry(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double tolerance) {
	 
	BOPAlgo_BOP aBOP;
	TopTools_ListOfShape otherShapes;
	otherShapes.Append(shape2);
	aBOP.AddArgument(shape1);
	aBOP.SetTools(otherShapes);
	aBOP.SetOperation(BOPAlgo_COMMON);
	aBOP.SetRunParallel(false);
	aBOP.SetNonDestructive(true);
	aBOP.SetFuzzyValue(tolerance);
	NProgressMonitor pi(10);

	TopoDS_Shape aR;
	aBOP.Perform(pi);
	aR = aBOP.Shape();
	if (aR.NbChildren() == 0)
		return false;
	else return true;
}