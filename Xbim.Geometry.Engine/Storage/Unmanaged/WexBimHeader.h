#pragma once
#include <ostream>
#include <gp_XYZ.hxx>


class WexBimHeader
{
public:
    WexBimHeader():
        ShapeCount(0),
        VertexCount(0),
        TriangleCount(0),
        MatrixCount(0),
        ProductCount(0),
        StyleCount(0),
        OneMeter(1.0f),
        RegionCount(0),
        MagicNumber(94132117),
        Version (4)//modified to contain local WCS where all data is relative to this root placement
    {}
    int MagicNumber;
    unsigned char Version;
    int ShapeCount;
    int VertexCount;
    int TriangleCount;
    int MatrixCount;
    int ProductCount;
    int StyleCount;
    float OneMeter;
    gp_XYZ LocalWCS;
    short RegionCount;
    void WriteToStream(std::ostream& strm);
};