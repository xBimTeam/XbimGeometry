#pragma once

#include <TopoDS_Shape.hxx> 
#include <BRepExtrema_ShapeProximity.hxx>
#include "NLoggingService.h" 



class NShapeProximityUtils
{
public:
	NShapeProximityUtils() {};
	int GetOverlappingSubShapes1Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection);
	int GetOverlappingSubShapes2Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection);
	bool IsOverlapping(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double tolerance, double linearDeflection, double angularDeflection);
private:
	bool HasOverlappingGeometry(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double percision);
};

