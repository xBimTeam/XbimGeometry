#include "WexBimStreamContent.h"

void WexBimStreamContent::updateHeader()
{
    //set up the header
    Header.MatrixCount = 0;
    Header.ShapeCount = 0;
    Header.TriangleCount = 0;
    Header.VertexCount = 0;
    for (auto& region : Regions)
    {
        Header.MatrixCount += region.MatrixCount();
        Header.ShapeCount += region.ShapeCount();
        Header.TriangleCount += region.TriangleCount();
        Header.VertexCount += region.VertexCount();
    }
    Header.ProductCount = (int)Products.size();
    Header.RegionCount = (short)Regions.Length(); 
    Header.StyleCount = Styles.NumberOfMaterials();
}


void WexBimStreamContent::WriteWexBimStream(std::ostream& strm)
{
    updateHeader();
    Header.WriteToStream(strm);
    for (auto& region: Regions) region.WriteToStream(strm);
    Styles.WriteToStream(strm);
    for (auto& productRec: Products) productRec.second.WriteToStream(strm);
    
    for (auto& region : Regions)
    {
        int modelsCount = (int)region.GeometryModels.size();
        strm.write((const char*) &modelsCount, sizeof(modelsCount));
        for (auto model: region.GeometryModels)
            model.WriteToStream(strm);
    }
}

WexBimProduct& WexBimStreamContent::AddProduct(int id, unsigned short typeId)
{
    WexBimProduct product(id, typeId);
    auto& inserted = Products.insert(std::make_pair(id, product));
    return inserted.first->second;
}
