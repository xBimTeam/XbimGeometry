#pragma once
#include <Standard_TypeDef.hxx>
#include <BRep_Builder.hxx>
#include <gp_Pnt2d.hxx>
#include <Precision.hxx>
struct KeyedPnt2d
{

public:
    KeyedPnt2d() :myPnt2d(), myID(-1) {}
    KeyedPnt2d(gp_XY pnt2d, int id) :myPnt2d(pnt2d), myID(id) {};
  
    //! Geometry point of this Vertex
    gp_XY myPnt2d;  
    Standard_Integer myID;

    //Makes a vertex with Precision = Confusion, we adjust this later when necessary
    TopoDS_Vertex CreateTopoVertex() const
    {
        BRep_Builder builder;
        TopoDS_Vertex vertex;
        builder.MakeVertex(vertex, gp_Pnt(myPnt2d.X(), myPnt2d.Y(), 0),Precision::Confusion());
        return vertex;
    }
};