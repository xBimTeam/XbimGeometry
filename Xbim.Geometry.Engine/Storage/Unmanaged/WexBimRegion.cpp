#include "WexBimRegion.h"

int WexBimRegion::MatrixCount()
{
	int count = 0;
	for (auto& geomModel : GeometryModels)
	{
		count += geomModel.MatrixCount();
	}
	return count;
}

int WexBimRegion::ShapeCount()
{
	int count = 0;
	for (auto& geomModel:  GeometryModels)
	{
		count += (int)geomModel.Shapes.size();
	}
	return count;
}

int WexBimRegion::TriangleCount()
{
	int count = 0;
	for (auto& geomModel : GeometryModels)
	{
		//srl, this is a fudge to allow webgl viewer to precreate arrays
		//it should not really consider the number of instances as the geometry should be created as an instance map, but webgl at time of coding did not support this
		count += geomModel.Mesh().TriangleCount() * (int)geomModel.Shapes.size();
	}
	return count;
}

int WexBimRegion::VertexCount()
{
	int count = 0;
	for (auto& geomModel : GeometryModels)
	{
		count += geomModel.Mesh().VertexCount();
	}
	return count;
}

int WexBimRegion::Population()
{
	return (int)GeometryModels.size();
}

void WexBimRegion::WriteToStream(std::ostream& strm)
{
	int populus = Population();
	strm.write((const char*)&populus, sizeof(populus));
	BVH_Vec3f centre = BndBox.Center();
	strm.write((const char*)&(centre.x()), sizeof(centre.x()));
	strm.write((const char*)&(centre.y()), sizeof(centre.y()));
	strm.write((const char*)&(centre.z()), sizeof(centre.z()));
	float xMin = BndBox.CornerMin().x(), yMin = BndBox.CornerMin().y(), zMin = BndBox.CornerMin().z();
	strm.write((const char*)&xMin, sizeof(xMin)); //Position followed by Size
	strm.write((const char*)&yMin, sizeof(yMin));
	strm.write((const char*)&zMin, sizeof(zMin));
	float xSize = BndBox.CornerMax().x() - xMin, ySize = BndBox.CornerMax().y() - yMin, zSize = BndBox.CornerMax().z() - zMin;
	strm.write((const char*)&xSize, sizeof(xSize)); //Position followed by Size
	strm.write((const char*)&ySize, sizeof(ySize));
	strm.write((const char*)&zSize, sizeof(zSize));
}


