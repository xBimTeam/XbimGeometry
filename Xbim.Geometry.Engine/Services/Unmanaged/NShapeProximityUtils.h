#pragma once

#include <TopoDS_Shape.hxx> 
#include <BRepExtrema_ShapeProximity.hxx>
#include "NLoggingService.h" 



class NShapeProximityUtils
{
public:
	static int GetOverlappingSubShapes1Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection);
	static int GetOverlappingSubShapes2Count(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection);
	static bool IsOverlapping(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double precision, double linearDeflection, double angularDeflection, bool includeTangents = true);
private:
	static bool HasOverlappingGeometry(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2, double percision);
};

