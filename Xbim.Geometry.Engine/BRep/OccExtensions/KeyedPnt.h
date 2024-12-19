#pragma once
#include <Standard_TypeDef.hxx>
#include <BRep_Builder.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>

struct KeyedPnt
{
public:
    KeyedPnt() :myPnt(), myID(-1) {};
    KeyedPnt(gp_XYZ pnt, int id) :myPnt(pnt), myID(id) {};
    KeyedPnt(const KeyedPnt& toCopy) :myPnt(toCopy.myPnt), myID(toCopy.myID) {}
    //! Geometry point of this Vertex
    gp_XYZ myPnt;
    Standard_Integer myID;

    //Makes a vertex with Precision = Confusion, we adjust this later when necessary
    TopoDS_Vertex CreateTopoVertex() const
    {
        BRep_Builder builder;
        TopoDS_Vertex vertex;
        builder.MakeVertex(vertex, gp_Pnt(myPnt.X(), myPnt.Y(), myPnt.Z()), Precision::Confusion());
        return vertex;
    }

};
