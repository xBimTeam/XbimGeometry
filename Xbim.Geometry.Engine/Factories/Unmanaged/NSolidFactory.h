#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"

#include <TopoDS_Solid.hxx>
#include <TopoDS_Builder.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <Geom_Curve.hxx>
class NSolidFactory
{
private:

	NLoggingService* pLoggingService;	
	TopoDS_Solid _emptySolid;

public:

	NSolidFactory()
	{		
		TopoDS_Builder builder;
		builder.MakeSolid(_emptySolid); //make an empty solid for failing operations
		pLoggingService = nullptr;
	};
	~NSolidFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Solid EmptySolid() { return _emptySolid; }
	TopoDS_Solid BuildBlock(gp_Ax2 ax2, double xLength, double yLength, double zLength);
	TopoDS_Solid BuildRectangularPyramid(gp_Ax2 ax2, double xLength, double yLength, double height);
	TopoDS_Solid BuildRightCircularCone(gp_Ax2 ax2, double radius, double height);
	TopoDS_Solid BuildRightCylinder(gp_Ax2 ax2, double radius, double height);
	TopoDS_Solid BuildSphere(gp_Ax2 ax2, double radius);
	TopoDS_Solid BuildSweptDiskSolid(const TopoDS_Wire& directrix, double radius, double innerRadius, BRepBuilderAPI_TransitionMode transitionMode);


};

