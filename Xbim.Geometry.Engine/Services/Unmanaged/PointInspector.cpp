#include "PointInspector.h"

Standard_EXPORT NCollection_CellFilter_Action PointInspector::Inspect(const Standard_Integer theID)
{
    const gp_XYZ& aXYZ = myPoints.Value(theID);
    const Standard_Real aSQDist = SquareDistance(myCurrent, aXYZ);
    if (aSQDist < mySQToler)
    {
        myResInd = theID;      
    }
    return CellFilter_Keep;
}
