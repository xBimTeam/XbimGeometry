#include "NSolidFactory.h"
#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <Geom_Circle.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <gp_Ax2.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>

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
		BRepPrimAPI_MakeCone coneMaker(ax2, radius, 0.0, height);
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

//if inner radius is not required it has a value of -1
TopoDS_Solid NSolidFactory::BuildSweptDiskSolid(const TopoDS_Wire& directrixWire, double radius, double innerRadius, BRepBuilderAPI_TransitionMode transitionMode)
{
	//the standard say
	
	//If the transitions between consecutive segments of the Directrix are not tangent continuous, the resulting solid is created by a miter at half angle between the two segments.
	//this will be the case for a polyline as each segment is not tangent continuous
	//composite urves will be tangent continuous
	try
	{
		//form the shape to sweep, the centre of the circles must be at the start of the directrix
		BRepBuilderAPI_MakeEdge edgeMaker;
		BRep_Builder builder;
		//get the normal at the start point
		gp_Vec dirAtStart;
		gp_Pnt startPoint;
		BRepAdaptor_CompCurve directrix(directrixWire, Standard_True);
		directrix.D1(0, startPoint, dirAtStart);
		gp_Ax2 axis(startPoint, dirAtStart);
		dirAtStart.Reverse(); //vector points in direction of directrix, need reversing for correct face orientation
		Handle(Geom_Circle) outer = new Geom_Circle(axis, radius);
		edgeMaker.Init(outer);
		TopoDS_Wire outerWire;
		builder.MakeWire(outerWire);
		builder.Add(outerWire, edgeMaker.Edge());

		BRepOffsetAPI_MakePipeShell oSweepMaker(directrixWire);
		oSweepMaker.SetTransitionMode(transitionMode);
		oSweepMaker.Add(outerWire);
	
		oSweepMaker.Build();
		if (oSweepMaker.IsDone())
		{
			//do we need an inner shell
			if (innerRadius > 0)
			{

				Handle(Geom_Circle) inner = new Geom_Circle(axis, innerRadius);
				edgeMaker.Init(inner);
				BRepBuilderAPI_MakeWire iWireMaker(edgeMaker.Edge());
				BRepOffsetAPI_MakePipeShell iSweepMaker(directrixWire);
				iSweepMaker.SetTransitionMode(transitionMode);
				TopoDS_Shape holeWire = iWireMaker.Wire().Reversed();
				iSweepMaker.Add(holeWire);
				iSweepMaker.Build();
				if (iSweepMaker.IsDone())
				{
					BRep_Builder builder;
					TopoDS_Solid solid;
					TopoDS_Shell shell;
					builder.MakeSolid(solid);
					builder.MakeShell(shell);
					TopExp_Explorer faceEx(oSweepMaker.Shape(), TopAbs_FACE);
					for (; faceEx.More(); faceEx.Next())
						builder.Add(shell, TopoDS::Face(faceEx.Current()));
					faceEx.Init(iSweepMaker.Shape(), TopAbs_FACE);
					for (; faceEx.More(); faceEx.Next())
						builder.Add(shell, TopoDS::Face(faceEx.Current()));
					//cap the faces
					TopoDS_Face startFace = BRepLib_MakeFace(TopoDS::Wire(oSweepMaker.FirstShape().Reversed()), Standard_True);
					builder.Add(startFace, iSweepMaker.FirstShape().Reversed());
					TopoDS_Face endFace = BRepLib_MakeFace (TopoDS::Wire(oSweepMaker.LastShape().Reversed()), Standard_True);
					builder.Add(endFace, iSweepMaker.LastShape().Reversed());
					builder.Add(shell, startFace);
					builder.Add(shell, endFace.Reversed());
					builder.Add(solid, shell);
					return solid;
				}
				else
				{
					pLoggingService->LogWarning("Could not build Inner radius of SweptDiskSolid");
					bool ok = oSweepMaker.MakeSolid();
					if (ok) return TopoDS::Solid(oSweepMaker.Shape());
				}

			}
			else
			{
				bool ok = oSweepMaker.MakeSolid();
				if (ok) return TopoDS::Solid(oSweepMaker.Shape());
			}
		}

	}
	catch (Standard_Failure e)
	{
		pLoggingService->LogError(e.GetMessageString());
	}
	pLoggingService->LogError("Could not build SweptDiskSolid");
	return _emptySolid;
}


