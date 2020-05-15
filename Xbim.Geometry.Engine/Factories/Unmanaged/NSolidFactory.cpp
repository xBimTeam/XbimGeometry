#include "NSolidFactory.h"
#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

TopoDS_Solid NSolidFactory::BuildBlock(gp_Ax2 ax2, double xLength, double yLength, double zLength)
{
	try
	{
		BRepPrimAPI_MakeBox boxMaker(ax2, xLength, yLength, zLength);
		return boxMaker.Solid(); //this builds the solid
	}
	catch (Standard_Failure e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
	}//but just in case we haven't
	pLoggingService->LogWarning("Could not build CsgBlock");
	return _emptySolid;
}

TopoDS_Solid NSolidFactory::BuildRectangularPyramid(gp_Ax2 ax2, double xLength, double yLength, double height)
{
	try
	{
		BRepPrimAPI_MakeWedge pyramidMaker(ax2, xLength, height, yLength, xLength / 2., yLength / 2., xLength / 2., yLength / 2.);
		return pyramidMaker.Solid(); //this builds the solid
	}
	catch (Standard_Failure e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
	}//but just in case we haven't
	pLoggingService->LogWarning("Could not build CsgRectangularPyramid");
	return _emptySolid;
}

TopoDS_Solid NSolidFactory::BuildRightCircularCone(gp_Ax2 ax2, double radius, double height)
{
	try
	{
		BRepPrimAPI_MakeCone coneMaker(ax2, radius, 0.0,  height);
		return coneMaker.Solid(); //this builds the solid
	}
	catch (Standard_Failure e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
	}//but just in case we haven't
	pLoggingService->LogWarning("Could not build CsgRightCircularCone");
	return _emptySolid;
}

TopoDS_Solid NSolidFactory::BuildRightCylinder(gp_Ax2 ax2, double radius, double height)
{
	try
	{
		BRepPrimAPI_MakeCylinder cylinderMaker(ax2, radius, height);
		return cylinderMaker.Solid(); //this builds the solid
	}
	catch (Standard_Failure e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
	}//but just in case we haven't
	pLoggingService->LogWarning("Could not build CsgRightCylinder");
	return _emptySolid;
}

TopoDS_Solid NSolidFactory::BuildSphere(gp_Ax2 ax2, double radius)
{
	try
	{
		BRepPrimAPI_MakeSphere sphereMaker(ax2, radius);
		return sphereMaker.Solid(); //this builds the solid
	}
	catch (Standard_Failure e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
	}//but just in case we haven't
	pLoggingService->LogWarning("Could not build CsgSphere");
	return _emptySolid;
}