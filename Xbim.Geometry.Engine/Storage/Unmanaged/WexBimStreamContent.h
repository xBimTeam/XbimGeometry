#pragma once
#include "WexBimProduct.h"
#include "WexBimRegion.h"
#include "WexBimStyles.h"
#include "WexBimHeader.h"
#include <map>

typedef NCollection_Vector<WexBimRegion> VectorOfWexBimRegion;
typedef std::map<int, WexBimProduct> WexBimProductLookup;
class WexBimStreamContent
{
private:
	void updateHeader();
public:
	WexBimStreamContent(const XCAFPrs_Style& theStyle) : Styles(theStyle) {}
	WexBimHeader					Header;
	VectorOfWexBimRegion			Regions;
	WexBimProductLookup				Products;
	WexBimStyles					Styles;
	void WriteWexBimStream(std::ostream& strm);
	WexBimRegion& AddRegion() { return Regions.Appended(); }
	WexBimProduct& AddProduct(int id, unsigned short typeId);
};