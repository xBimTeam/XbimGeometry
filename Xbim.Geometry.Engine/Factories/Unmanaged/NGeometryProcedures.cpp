#include "NGeometryProcedures.h"

TopLoc_Location NGeometryProcedures::ToLocation(gp_Pnt2d location, gp_XY xDirection)
{
	try
	{
		//Direction is not created if it has null magnitude					
		static double res2 = gp::Resolution() * gp::Resolution();
		double aMagnitude = xDirection.SquareModulus();
		if (aMagnitude < res2)
			xDirection = gp_XY(1, 0); //reset to default
		gp_Ax2d axis2d(location, xDirection);
		gp_Trsf2d trsf;
		trsf.SetTransformation(axis2d);
		trsf.Invert();
		return TopLoc_Location(trsf);
	}
	catch (const Standard_Failure& e)
	{
		std::stringstream strm;
		e.Print(strm);
		pLoggingService->LogError(strm.str().c_str());
	}

	pLoggingService->LogWarning("Could not axis placement");
	return TopLoc_Location();
}
