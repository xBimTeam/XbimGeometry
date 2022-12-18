#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Compound.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopTools_SequenceOfShape.hxx>
class NProfileFactory : public NFactoryBase
{
private:
	Handle(Geom_Plane) _xyPlane;
public:
	NProfileFactory()
	{
		_xyPlane = new Geom_Plane(gp::XOY());
	}
	
	TopoDS_Compound MakeCompound(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
	//Build a face and an outer bound, as profile defs are 2d  the xy plane is assumed for the surface
	TopoDS_Face MakeFace(const TopoDS_Wire& wire);
	//Build a face and an outer bound and inner wire loops, as profile defs are 2d the xy plane is assumed for the surface
	TopoDS_Face MakeFace(const TopoDS_Wire& wire, const TopTools_SequenceOfShape& innerLoops);
};

