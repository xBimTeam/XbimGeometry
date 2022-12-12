#include "WexBimGeometryModel.h"


int WexBimGeometryModel::MatrixCount()
{
	if (Shapes.size() > 1) return (int)Shapes.size(); else return 0;
}

void WexBimGeometryModel::WriteToStream(std::ostream& strm)
{
    //in this implementation we only have a single instance of every geometry, maps are not supported due to workflow contraints 
    // it would force us to process models by Type not by Product. Might need to revisit this for future Type centric workflows
    int shapecount = (int) Shapes.size();
    strm.write((const char *)&shapecount, sizeof(shapecount));
    for (auto& shape: Shapes)
    {
		shape.WriteToStream(strm, shapecount>1);
    }

	std::streampos geomLengthPos = strm.tellp();
	int geomLength = 0;
	strm.write((const char*)&geomLength, sizeof(geomLength)); //reserve the space
	std::streampos geomStartPos = strm.tellp();
	mesh.WriteToStream(strm);
	std::streampos geomEndPos = strm.tellp();

	geomLength = (int)(geomEndPos - geomStartPos);
	geomStartPos -= sizeof(geomLength);
	strm.seekp(geomStartPos);
	strm.write((const char*)&geomLength, sizeof(geomLength));
	strm.seekp(geomEndPos);
}
