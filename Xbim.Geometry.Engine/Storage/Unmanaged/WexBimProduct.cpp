#include "WexBimProduct.h"

void WexBimProduct::WriteToStream(std::ostream& strm)
{
	strm.write((const char*)&ProductId, sizeof(ProductId));
	strm.write((const char*)&ProductType, sizeof(ProductType));
	
	float xMin = BndBox.CornerMin().x(), yMin = BndBox.CornerMin().y(), zMin = BndBox.CornerMin().z();
	strm.write((const char*)&xMin, sizeof(xMin)); //Position 
	strm.write((const char*)&yMin, sizeof(yMin));
	strm.write((const char*)&zMin, sizeof(zMin));
	float xSize = BndBox.CornerMax().x() - xMin, ySize = BndBox.CornerMax().y() - yMin, zSize = BndBox.CornerMax().z() - zMin;
	strm.write((const char*)&xSize, sizeof(xSize)); //Size
	strm.write((const char*)&ySize, sizeof(ySize));
	strm.write((const char*)&zSize, sizeof(zSize));
}