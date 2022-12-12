#pragma once

#include "Graphic3d_BndBox3f.h"

#include <ostream>


class WexBimProduct
{
public:
	WexBimProduct() :ProductId(-1), ProductType(-1) {}
	WexBimProduct(int productId, unsigned short productTypeId) :ProductId(productId), ProductType(productTypeId) {}
	int						ProductId;
	unsigned short			ProductType;
	Graphic3d_BndBox3f      BndBox;        //!< bounding box

	void WriteToStream(std::ostream& strm);
	bool IsUndefined() { return ProductId == -1; }
	//! Returns True if styles are the same
	//! Methods for using Style as key in maps
	Standard_Boolean IsEqual(const WexBimProduct& theOther) const
	{
		return (ProductId == theOther.ProductId && ProductType == theOther.ProductType);
	}

	//! Returns True if styles are the same.
	Standard_Boolean operator== (const WexBimProduct& theOther) const
	{
		return IsEqual(theOther);
	}


	static Standard_Integer HashCode(const WexBimProduct& theProduct, const Standard_Integer theUpperBound)
	{
		Standard_Integer theValue = theProduct.ProductId;
		return  IntegerHashCode(theValue, IntegerLast(), theUpperBound);;
	}

	//! Returns True when the two keys are the same.
	static Standard_Boolean IsEqual(const WexBimProduct& theP1, const WexBimProduct& theP2)
	{
		return theP1.IsEqual(theP2);
	}

};