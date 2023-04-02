#pragma once
#include "NFactoryBase.h"
#include <BRep_Builder.hxx>
#include <gp_Pln.hxx>
#include <TopTools_SequenceOfShape.hxx>

class NFaceFactory : public NFactoryBase
{
public:	
	TopoDS_Face BuildProfileDef(gp_Pln plane, const TopoDS_Wire& wire);
	gp_Vec Normal(const TopoDS_Face& face);
	TopoDS_Face BuildFace(Handle(Geom_Surface) surface, double tolerance);
	TopoDS_Face BuildFace(Handle(Geom_Surface) surface, const TopoDS_Wire& outerLoop, const TopTools_SequenceOfShape& innerLoops, double tolerance, bool sameSense);
	bool AddParameterisedCurves(Handle(Geom_Surface)& surface, const TopoDS_Wire& wire, double tolerance);
};

