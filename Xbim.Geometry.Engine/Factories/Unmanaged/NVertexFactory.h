#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Vertex.hxx>
#include <BRep_Builder.hxx>
#include <Precision.hxx>
class NVertexFactory : public NFactoryBase
{
public:
	BRep_Builder builder;
	TopoDS_Vertex BuildVertex(double x, double y, double z, double precision = Precision::Confusion());
};

