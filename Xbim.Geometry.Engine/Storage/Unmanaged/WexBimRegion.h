#pragma once
#include "Graphic3d_BndBox3f.h"
#include "WexBimGeometryModel.h"
#include <vector>
typedef std::vector<WexBimGeometryModel> VectorOfWexBimGeometryModel;

class WexBimRegion
{
public:
	Graphic3d_BndBox3f				    BndBox;        //!< bounding box
	VectorOfWexBimGeometryModel			GeometryModels;
	int MatrixCount();
	int ShapeCount();
	int TriangleCount();
	int VertexCount();
	int Population();
	void WriteToStream(std::ostream& strm);
	
	
};
