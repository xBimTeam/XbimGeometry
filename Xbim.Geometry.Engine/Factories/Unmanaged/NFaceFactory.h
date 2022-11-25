#pragma once
#include "NFactoryBase.h"
#include <BRep_Builder.hxx>
#include <gp_Pln.hxx>

class NFaceFactory : public NFactoryBase
{
public:	
	TopoDS_Face BuildProfileDef(gp_Pln plane, const TopoDS_Wire& wire);
	gp_Vec Normal(const TopoDS_Face& face);
};

