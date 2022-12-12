#include "WexBimHeader.h"
void WexBimHeader::WriteToStream(std::ostream& strm)
{
    strm.write((const char*)&MagicNumber, sizeof(MagicNumber));
    strm.write((const char*)&Version, sizeof(Version));
    strm.write((const char*)&ShapeCount, sizeof(ShapeCount));
    strm.write((const char*)&VertexCount, sizeof(VertexCount));
    strm.write((const char*)&TriangleCount, sizeof(TriangleCount));
    strm.write((const char*)&MatrixCount, sizeof(MatrixCount));
    strm.write((const char*)&ProductCount, sizeof(ProductCount));
    strm.write((const char*)&StyleCount, sizeof(StyleCount));
    strm.write((const char*)&OneMeter, sizeof(OneMeter));
    // local WCS
    double x = LocalWCS.X(), y = LocalWCS.Y(), z = LocalWCS.Z();
    strm.write((const char*)&x, sizeof(x) );
    strm.write((const char*)&y, sizeof(y));
    strm.write((const char*)&z, sizeof(z));

    strm.write((const char*)&RegionCount, sizeof(RegionCount));

}


