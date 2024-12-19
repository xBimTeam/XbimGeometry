#include "NVertexFactory.h"

TopoDS_Vertex NVertexFactory::BuildVertex(double x, double y, double z, double precision)
{
    TopoDS_Vertex v;
    gp_Pnt pnt(x, y, z);
    builder.MakeVertex(v, pnt,precision);
    return v;
}
