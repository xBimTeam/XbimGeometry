#pragma once

#include <TopoDS_Shape.hxx> 
#include "NLoggingService.h" 



class NCollisionDetectionService
{
public:
	static bool IsColliding(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection);
};

