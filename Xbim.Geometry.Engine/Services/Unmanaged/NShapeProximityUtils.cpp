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

bool NShapeProximityUtils::IsOverlapping(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double percision, double linearDeflection, double angularDeflection, bool includeTangents)
{
	BRepMesh_IncrementalMesh aMesh1(shape1, linearDeflection, Standard_False, angularDeflection);
	BRepMesh_IncrementalMesh aMesh2(shape2, linearDeflection, Standard_False, angularDeflection);

	BRepExtrema_ShapeProximity proximity(shape1, shape2, percision);
	proximity.Perform();

	if (proximity.IsDone()) {

		bool overlapping = proximity.OverlapSubShapes1().Size() > 0 && 
				proximity.OverlapSubShapes2().Size() > 0;

		//TODO (Ibrahim): better tangency check
		// if we need to discard tangents we need to check overllaping volume
		// the algorithm cant be run with negative tolerance
		if (overlapping && !includeTangents) {
				return NShapeProximityUtils::HasOverlappingGeometry(shape1, shape2, percision);
		}

		return overlapping;
	}

	return false;
}

bool NShapeProximityUtils::HasOverlappingGeometry(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double percision) {
	 
	BOPAlgo_BOP aBOP;
	TopTools_ListOfShape otherShapes;
	otherShapes.Append(shape2);
	aBOP.AddArgument(shape1);
	aBOP.SetTools(otherShapes);
	aBOP.SetOperation(BOPAlgo_COMMON);
	aBOP.SetRunParallel(false);
	aBOP.SetNonDestructive(true);
	aBOP.SetFuzzyValue(percision);
	NProgressMonitor pi(10);

	TopoDS_Shape aR;
	aBOP.Perform(pi);
	aR = aBOP.Shape();
	if (aR.NbChildren() == 0)
		return false;
	else return true;
}