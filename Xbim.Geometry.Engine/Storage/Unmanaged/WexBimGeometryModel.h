#pragma once
#include "WexBimShape.h"
#include <NCollection_IndexedDataMap.hxx>


#include <TopLoc_Location.hxx>
#include<Precision.hxx>
#include "NWexBimMesh.h"



class WexBimGeometryModel
{
private:

	NWexBimMesh mesh;
public:
	WexBimGeometryModel(double precision) : mesh(precision){}
	//TCollection_AsciiString MeshId;
	std::vector<WexBimShape> Shapes;
	
	NWexBimMesh& Mesh() { return mesh; };
	int MatrixCount();
	void WriteToStream(std::ostream& strm);
};