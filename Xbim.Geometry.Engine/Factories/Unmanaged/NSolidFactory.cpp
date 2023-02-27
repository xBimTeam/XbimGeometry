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
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepCheck_Shell.hxx>
#include <ShapeFix_Shell.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepCheck_Status.hxx>
#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepCheck_Solid.hxx>
#include <BRepTools_WireExplorer.hxx>
#include "NCurveFactory.h"
#include <math.h>

bool NSolidFactory::TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape)
{
	try
	{
		BRepCheck_Solid checker(solid);
		BRepCheck_ListOfStatus st = checker.Status();
		if (st.Size() == 1 && st.First() == BRepCheck_NoError)
		{
			shape = solid;
			shape.Checked(true);
			return false; //no upgrade required
		}
		//The code below is merely designed to identify incorrectly built solids, these are solids built under xbim code control, the fix to the error should be in the builder code, not here, 
		//ShapeFix_ solutions are viable but not performant and ideally the underlying cause of the problem should be fixed
		//If the problem is badly authored user models or BIM exporters, they are fixable by ShapeFix_ solutions but the results are not always ideal
		//The one grey are is solids that comprise multiple solids, the IFC schema indicates this is incorrect but several BIM tools do this

		std::stringstream errMsg;
		errMsg << "TryUpgrade: Error forming solid = ";
		for (auto& it = st.cbegin(); it != st.cend(); it++)
		{

			switch (*it)
			{
				/*	case BRepCheck_NoError:
						break;*/
			case BRepCheck_InvalidPointOnCurve:
				errMsg << "BRepCheck_InvalidPointOnCurve ";
				break;
			case BRepCheck_InvalidPointOnCurveOnSurface:
				errMsg << "BRepCheck_InvalidPointOnCurveOnSurface ";
				break;
			case BRepCheck_InvalidPointOnSurface:
				errMsg << "BRepCheck_InvalidPointOnSurface ";
				break;
			case BRepCheck_No3DCurve:
				errMsg << "BRepCheck_No3DCurve ";
				break;
			case BRepCheck_Multiple3DCurve:
				errMsg << "BRepCheck_Multiple3DCurve ";
				break;
			case BRepCheck_Invalid3DCurve:
				errMsg << "BRepCheck_Invalid3DCurve ";
				break;
			case BRepCheck_NoCurveOnSurface:
				errMsg << "BRepCheck_NoCurveOnSurface ";
				break;
			case BRepCheck_InvalidCurveOnSurface:
				errMsg << "BRepCheck_InvalidCurveOnSurface ";
				break;
			case BRepCheck_InvalidCurveOnClosedSurface:
				errMsg << "BRepCheck_InvalidCurveOnClosedSurface ";
				break;
			case BRepCheck_InvalidSameRangeFlag:
				errMsg << "BRepCheck_InvalidSameRangeFlag ";
				break;
			case BRepCheck_InvalidSameParameterFlag:
				errMsg << "BRepCheck_InvalidSameParameterFlag ";
				break;
			case BRepCheck_InvalidDegeneratedFlag:
				errMsg << "BRepCheck_InvalidDegeneratedFlag ";
				break;
			case BRepCheck_FreeEdge:
				errMsg << "BRepCheck_FreeEdge ";
				break;
			case BRepCheck_InvalidMultiConnexity:
				errMsg << "BRepCheck_InvalidMultiConnexity ";
				break;
			case BRepCheck_InvalidRange:
				errMsg << "BRepCheck_InvalidRange ";
				break;
			case BRepCheck_EmptyWire:
				errMsg << "BRepCheck_EmptyWire ";
				break;
			case BRepCheck_RedundantEdge:
				errMsg << "BRepCheck_RedundantEdge ";
				break;
			case BRepCheck_SelfIntersectingWire:
				errMsg << "BRepCheck_SelfIntersectingWire ";
				break;
			case BRepCheck_NoSurface:
				errMsg << "BRepCheck_NoSurface ";
				break;
			case BRepCheck_InvalidWire:
				errMsg << "BRepCheck_InvalidWire ";
				break;
			case BRepCheck_RedundantWire:
				errMsg << "BRepCheck_RedundantWire ";
				break;
			case BRepCheck_IntersectingWires:
				errMsg << "BRepCheck_IntersectingWires ";
				break;
			case BRepCheck_InvalidImbricationOfWires:
				errMsg << "BRepCheck_InvalidImbricationOfWires ";
				break;
			case BRepCheck_EmptyShell:
				errMsg << "BRepCheck_EmptyShell ";
				break;
			case BRepCheck_RedundantFace:
				errMsg << "BRepCheck_RedundantFace ";
				break;
			case BRepCheck_InvalidImbricationOfShells:
				errMsg << "BRepCheck_InvalidImbricationOfShells ";
				break;
			case BRepCheck_UnorientableShape:
				errMsg << "BRepCheck_UnorientableShape ";
				break;
			case BRepCheck_NotClosed:
				errMsg << "BRepCheck_NotClosed ";
				break;
			case BRepCheck_NotConnected:
				errMsg << "BRepCheck_NotConnected ";
				break;
			case BRepCheck_SubshapeNotInShape:
				errMsg << "BRepCheck_SubshapeNotInShape ";
				break;
			case BRepCheck_BadOrientation:
				errMsg << "BRepCheck_BadOrientation ";
				break;
			case BRepCheck_BadOrientationOfSubshape:
				errMsg << "BRepCheck_BadOrientationOfSubshape ";
				break;
			case BRepCheck_InvalidPolygonOnTriangulation:
				errMsg << "BRepCheck_InvalidPolygonOnTriangulation ";
				break;
			case BRepCheck_InvalidToleranceValue:
				errMsg << "BRepCheck_InvalidToleranceValue ";
				break;
			case BRepCheck_EnclosedRegion:
				errMsg << "BRepCheck_EnclosedRegion ";
				break;
			case BRepCheck_CheckFail:
				errMsg << "BRepCheck_CheckFail ";
				break;
			default:
				errMsg << "BRepCheck_Undefined ";
				break;
			}
			//over time we will need to handle all these errors but for now just identifying them
			Standard_Failure::Raise(errMsg);
		}
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Could not build a Solid from a Shell");
	return true; //return  upgrade is needed
}


TopoDS_Solid NSolidFactory::BuildBlock(gp_Ax2 ax2, double xLength, double yLength, double zLength)
{
	try
	{
		BRepPrimAPI_MakeBox boxMaker(ax2, xLength, yLength, zLength);
		return boxMaker.Solid(); //this builds the solid
	}
	catch (const Standard_Failure& e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
		LogStandardFailure(e);
	}//but just in case we haven't
	pLoggingService->LogError("Could not build CsgBlock");
	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::BuildRectangularPyramid(gp_Ax2 ax2, double xLength, double yLength, double height)
{
	try
	{
		double xOff = xLength / 2;
		double yOff = yLength / 2;
		double precision = Precision::Confusion();
		gp_Pnt bl(0, 0, 0);
		gp_Pnt br(xLength, 0, 0);
		gp_Pnt tr(xLength, yLength, 0);
		gp_Pnt tl(0, yLength, 0);
		gp_Pnt p(xOff, yOff, height);
		//make the vertices
		BRep_Builder builder;
		TopoDS_Vertex vbl, vbr, vtr, vtl, vp;
		TopoDS_Shell shell;
		builder.MakeShell(shell);
		builder.MakeVertex(vbl, bl, precision);
		builder.MakeVertex(vbr, br, precision);
		builder.MakeVertex(vtr, tr, precision);
		builder.MakeVertex(vtl, tl, precision);
		builder.MakeVertex(vp, p, precision);
		//make the edges
		TopoDS_Wire baseWire;
		builder.MakeWire(baseWire);
		const TopoDS_Edge& brbl = BRepBuilderAPI_MakeEdge(vbr, vbl);
		const TopoDS_Edge& trbr = BRepBuilderAPI_MakeEdge(vtr, vbr);
		const TopoDS_Edge& tltr = BRepBuilderAPI_MakeEdge(vtl, vtr);
		const TopoDS_Edge& bltl = BRepBuilderAPI_MakeEdge(vbl, vtl);
		builder.Add(baseWire, brbl);
		builder.Add(baseWire, bltl);
		builder.Add(baseWire, tltr);
		builder.Add(baseWire, trbr);
		BRepBuilderAPI_MakeFace afaceBlder(gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, -1)), baseWire, Standard_True);
		builder.Add(shell, afaceBlder.Face());
		//build the sides
		const TopoDS_Edge& blp = BRepBuilderAPI_MakeEdge(vbl, vp);
		const TopoDS_Edge& tlp = BRepBuilderAPI_MakeEdge(vtl, vp);
		const TopoDS_Edge& brp = BRepBuilderAPI_MakeEdge(vbr, vp);
		const TopoDS_Edge& trp = BRepBuilderAPI_MakeEdge(vtr, vp);
		TopoDS_Wire bltlWire;
		builder.MakeWire(bltlWire);
		builder.Add(bltlWire, bltl.Reversed());
		builder.Add(bltlWire, blp);
		builder.Add(bltlWire, tlp.Reversed());
		BRepBuilderAPI_MakeFace bfaceBlder(bltlWire, Standard_True);
		builder.Add(shell, bfaceBlder.Face());

		TopoDS_Wire tltrWire;
		builder.MakeWire(tltrWire);
		builder.Add(tltrWire, tltr.Reversed());
		builder.Add(tltrWire, tlp);
		builder.Add(tltrWire, trp.Reversed());
		BRepBuilderAPI_MakeFace cfaceBlder(tltrWire, Standard_True);
		builder.Add(shell, cfaceBlder.Face());


		TopoDS_Wire brtlWire;
		builder.MakeWire(brtlWire);
		builder.Add(brtlWire, trbr.Reversed());
		builder.Add(brtlWire, trp);
		builder.Add(brtlWire, brp.Reversed());
		BRepBuilderAPI_MakeFace dfaceBlder(brtlWire, Standard_True);
		builder.Add(shell, dfaceBlder.Face());

		TopoDS_Wire blbrWire;
		builder.MakeWire(blbrWire);
		builder.Add(blbrWire, brbl.Reversed());
		builder.Add(blbrWire, brp);
		builder.Add(blbrWire, blp.Reversed());
		BRepBuilderAPI_MakeFace efaceBlder(blbrWire, Standard_True);
		builder.Add(shell, efaceBlder.Face());

		BRepBuilderAPI_MakeSolid solidMaker(shell);

		return solidMaker.Solid(); //this builds the solid
	}
	catch (const Standard_Failure& e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
		LogStandardFailure(e);
	}//but just in case we haven't
	pLoggingService->LogError("Could not build CsgRectangularPyramid");
	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::BuildRightCircularCone(gp_Ax2 ax2, double radius, double height)
{
	try
	{
		BRepPrimAPI_MakeCone coneMaker(ax2, radius, 0.0, height);
		return coneMaker.Solid(); //this builds the solid
	}
	catch (const Standard_Failure& e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
		LogStandardFailure(e);
	}//but just in case we haven't
	pLoggingService->LogError("Could not build CsgRightCircularCone");
	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::BuildRightCylinder(gp_Ax2 ax2, double radius, double height)
{
	try
	{
		BRepPrimAPI_MakeCylinder cylinderMaker(ax2, radius, height);
		return cylinderMaker.Solid(); //this builds the solid
	}
	catch (const Standard_Failure& e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
		LogStandardFailure(e);
	}//but just in case we haven't
	pLoggingService->LogError("Could not build CsgRightCylinder");
	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::BuildSphere(gp_Ax2 ax2, double radius)
{
	try
	{
		BRepPrimAPI_MakeSphere sphereMaker(ax2, radius);
		return sphereMaker.Solid(); //this builds the solid
	}
	catch (const Standard_Failure& e) //pretty much only throws Standard_DomainError, we should have got most of these earlier
	{
		LogStandardFailure(e);
	}//but just in case we haven't
	pLoggingService->LogError("Could not build CsgSphere");
	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::BuildSweptDiskSolid(const Handle(Geom_Curve) directrixCurve, double radius, double innerRadius)
{

	try
	{
		BRepBuilderAPI_MakeEdge edgeMaker(directrixCurve);
		BRepBuilderAPI_MakeWire wireMaker(edgeMaker.Edge());
		return BuildSweptDiskSolid(wireMaker.Wire(), radius, innerRadius);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
		pLoggingService->LogError("Could not build SweptDiskSolid directrix");
	}

	return TopoDS_Solid();
}

//if inner radius is not required it has a value of -1
TopoDS_Solid NSolidFactory::BuildSweptDiskSolid(const TopoDS_Wire& directrixWire, double radius, double innerRadius)
{
	//the standard say

	//If the transitions between consecutive segments of the Directrix are not tangent continuous, the resulting solid is created by a miter at half angle between the two segments.
	//this will be the case for a polyline as each segment is not tangent continuous
	//composite urves will be tangent continuous
	try
	{
		bool haveFirstEdge = false;
		//get the normal at the start point
		gp_Vec dirAtStart;
		gp_Pnt startPoint;
		int numPeriodics = 0;
		int numEdges = 0;
		bool hasConsecutiveNonPeriodics = false;
		bool previousEdgeIsPerodic = false;;
		for (BRepTools_WireExplorer wireExplorer(directrixWire); wireExplorer.More(); wireExplorer.Next())
		{
			numEdges++;
			double start, end;
			auto edge = wireExplorer.Current();
			auto curve = BRep_Tool::Curve(edge, start, end);
			auto basisCurve = NCurveFactory::GetBasisCurve(curve);
			if (basisCurve->IsPeriodic())
			{
				numPeriodics++;
				previousEdgeIsPerodic = true;
			}
			else if (haveFirstEdge && !previousEdgeIsPerodic)
			{
				hasConsecutiveNonPeriodics = true;
			}
			else
				previousEdgeIsPerodic = false;
			if (!haveFirstEdge)
			{
				haveFirstEdge = true;
				curve->D1(0, startPoint, dirAtStart);
			}

		}

		BRepBuilderAPI_TransitionMode transitionMode = BRepBuilderAPI_TransitionMode::BRepBuilderAPI_RightCorner; //lines sweep better with right corner sweeping
		if (numPeriodics > 0)
		{
			transitionMode = BRepBuilderAPI_TransitionMode::BRepBuilderAPI_Transformed; //curves only sweep if we use a transform
			//if (hasConsecutiveNonPeriodics) //this cannot be correctly swept as a pipe with contant radius
			//	LogStandardFailure( "Detected incorrect definition of directrix for IfcSweptDiskSolid. The swept disk will be an incorrect shape");
		}

		//form the shape to sweep, the centre of the circles must be at the start of the directrix
		BRepBuilderAPI_MakeEdge edgeMaker;
		BRep_Builder outerBuilder;

		gp_Ax2 axis(startPoint, dirAtStart);
		dirAtStart.Reverse(); //vector points in direction of directrix, need reversing for correct face orientation
		Handle(Geom_Circle) outer = new Geom_Circle(axis, radius);
		edgeMaker.Init(outer);
		TopoDS_Wire outerWire;
		outerBuilder.MakeWire(outerWire);
		outerBuilder.Add(outerWire, edgeMaker.Edge());

		BRepOffsetAPI_MakePipeShell oSweepMaker(directrixWire);
		oSweepMaker.SetTransitionMode(transitionMode);
		oSweepMaker.Add(outerWire);

		oSweepMaker.Build();
		if (oSweepMaker.IsDone())
		{
			//do we need an inner shell
			if (!isnan(innerRadius))
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
					BRep_Builder innerBuilder;
					TopoDS_Solid solid;
					TopoDS_Shell shell;
					innerBuilder.MakeSolid(solid);
					innerBuilder.MakeShell(shell);
					TopExp_Explorer faceEx(oSweepMaker.Shape(), TopAbs_FACE);
					for (; faceEx.More(); faceEx.Next())
						innerBuilder.Add(shell, TopoDS::Face(faceEx.Current()));
					faceEx.Init(iSweepMaker.Shape(), TopAbs_FACE);
					for (; faceEx.More(); faceEx.Next())
						innerBuilder.Add(shell, TopoDS::Face(faceEx.Current()));
					//cap the faces
					TopoDS_Face startFace = BRepLib_MakeFace(TopoDS::Wire(oSweepMaker.FirstShape().Reversed()), Standard_True);
					innerBuilder.Add(startFace, iSweepMaker.FirstShape().Reversed());
					TopoDS_Face endFace = BRepLib_MakeFace(TopoDS::Wire(oSweepMaker.LastShape().Reversed()), Standard_True);
					innerBuilder.Add(endFace, iSweepMaker.LastShape().Reversed());
					innerBuilder.Add(shell, startFace);
					innerBuilder.Add(shell, endFace.Reversed());
					innerBuilder.Add(solid, shell);
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
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	catch (...)
	{
		pLoggingService->LogError("Could not build SweptDiskSolid");
	}

	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::BuildExtrudedAreaSolid(const TopoDS_Face& face, gp_Dir extrudeDirection, double depth, const TopLoc_Location& location)
{
	try
	{
		gp_Vec extrusionVec(extrudeDirection);
		extrusionVec.Multiply(depth);
		BRepPrimAPI_MakePrism prismMaker(face, extrusionVec);
		if (!prismMaker.IsDone()) Standard_Failure::Raise("Error extruding prism");
		TopoDS_Solid extrusion = TopoDS::Solid(prismMaker.Shape());
		if (!location.IsIdentity())
			extrusion.Move(location);
		return extrusion;
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Could not build ExtrudedAreaSolid");
	return TopoDS_Solid();
}

/// <summary>
/// This method builds a solid but it might not be a topologically correct solid
/// </summary>
/// <param name="shell"></param>
/// <param name="resultSolid"></param>
TopoDS_Solid NSolidFactory::MakeSolid(const TopoDS_Shell& shell)
{
	BRep_Builder builder;
	TopoDS_Solid resultSolid;
	builder.MakeSolid(resultSolid);
	builder.Add(resultSolid, shell);
	//resultSolid.Checked(true);
	resultSolid.Closed(true);
	return resultSolid;
}

TopoDS_Solid NSolidFactory::CastToSolid(const TopoDS_Shape& shape)
{
	try 
	{
		return TopoDS::Solid(shape);
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Could not cast to Solid");
	return TopoDS_Solid();
}

TopoDS_Solid NSolidFactory::MakeSweptSolid(const TopoDS_Face& face, const gp_Vec& direction)
{
	try
	{
		BRepPrimAPI_MakePrism prismMaker(face, direction);
		return TopoDS::Solid(prismMaker.Shape()); //this will throw exceptions if it fails which will be caught and reported
	}
	catch (const Standard_Failure& e)
	{
		LogStandardFailure(e);
	}
	pLoggingService->LogError("Could not build ExtrudedAreaSolid");
	return TopoDS_Solid();
}
