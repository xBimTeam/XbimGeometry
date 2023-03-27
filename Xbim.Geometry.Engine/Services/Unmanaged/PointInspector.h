#pragma once
#include <Standard_TypeDef.hxx>
#include <gp_Pnt.hxx>
#include <TColStd_ListOfInteger.hxx>
#include <NCollection_Vector.hxx>
#include <NCollection_CellFilter.hxx>
typedef NCollection_Vector<gp_XYZ> VectorOfXYZ;

struct CellFilter_InspectorXYZ
{

	//! Points dimension
	enum { Dimension = 3 };

	//! Points type
	typedef gp_XYZ Point;

	//! Access to co-ordinate
	static Standard_Real Coord(int i, const Point& thePnt) { return thePnt.Coord(i + 1); }

	//! Auxiliary method to shift point by each coordinate on given value;
	//! useful for preparing a points range for Inspect with tolerance
	gp_XYZ Shift(const gp_XYZ& thePnt, Standard_Real theTol) const
	{
		return gp_XYZ(thePnt.X() + theTol, thePnt.Y() + theTol, thePnt.Z() + theTol);
	}
};

class PointInspector : public CellFilter_InspectorXYZ
{
public:
	typedef Standard_Integer Target;
	//! Constructor; remembers the tolerance
	PointInspector(const Standard_Real theTol) : mySQToler(theTol* theTol)
	{}

	//! Keep the points used for comparison
	int Add(const gp_XYZ& thePnt)
	{
		myPoints.Append(thePnt);
		return myPoints.Length() - 1; //0 based index
	}


	//! Set current point to search for coincidence
	void SetCurrent(const gp_XYZ& theCurPnt)
	{
		myCurrent = theCurPnt;
		myResInd = -1;
	}

	//! index of point coinciding
	const int ResInd()
	{
		return myResInd;
	}

	inline Standard_Real SquareDistance(const gp_XYZ& a, const gp_XYZ& b) const
	{
		Standard_Real d = 0, dd;
		dd = a.X(); dd -= b.X(); dd *= dd; d += dd;
		dd = a.Y(); dd -= b.Y(); dd *= dd; d += dd;
		dd = a.Z(); dd -= b.Z(); dd *= dd; d += dd;
		return(d);
	}
	//! Implementation of inspection method
	Standard_EXPORT NCollection_CellFilter_Action Inspect(const Standard_Integer theTarget);
	VectorOfXYZ myPoints;
	VectorOfXYZ myNormals;
	Standard_Real mySQToler;
private:

	int myResInd = -1;
	gp_XYZ myCurrent;
};
