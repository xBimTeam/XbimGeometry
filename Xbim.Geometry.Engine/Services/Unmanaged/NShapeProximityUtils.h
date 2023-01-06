#pragma once

#include <TopoDS_Shape.hxx> 
#include "NLoggingService.h" 



class NShapeProximityUtils
{
public:
	static int GetOverlappingSubShapesCount(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection);
};

