#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Compound.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <Geom2d_Curve.hxx>
#include <gp_Ax22d.hxx>
class NProfileFactory : public NFactoryBase
{
private:
	Handle(Geom_Plane) _xyPlane;
public:
	NProfileFactory()
	{
		_xyPlane = new Geom_Plane(gp::XOY());
	}

	TopoDS_Compound MakeCompound(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
	//Build a face and an outer bound, as profile defs are 2d  the xy plane is assumed for the surface
	TopoDS_Face MakeFace(const TopoDS_Wire& wire);
	TopoDS_Wire ValidateAndFixProfileWire(const TopoDS_Wire& wire);
	TopoDS_Edge MakeEdge(const gp_Pnt& start, const gp_Pnt& end);
	TopoDS_Edge MakeEdge(const gp_Pnt2d& start, const gp_Pnt2d& end);
	TopoDS_Edge MakeEdge(const Handle(Geom_Curve)& hCurve);
	TopoDS_Edge MakeEdge(const Handle(Geom2d_Curve)& hCurve2d);
	TopoDS_Wire MakeWire(const TopoDS_Edge& edge);
	TopoDS_Wire MakeWire(const TopTools_ListOfShape& edges);
	TopoDS_Face MakeFace(const TopoDS_Wire& outer, const TopoDS_Wire& inner);
	//Build a face and an outer bound and inner wire loops, as profile defs are 2d the xy plane is assumed for the surface
	TopoDS_Face MakeFace(const TopoDS_Wire& wire, const TopTools_SequenceOfShape& innerLoops);
	TopoDS_Wire BuildRectangle(double dimX, double dimY, const TopLoc_Location& location);
	TopoDS_Face BuildRectangleHollowProfileDef(const TopLoc_Location& location, double xDim, double yDim, double wallThickness, double outerFilletRadius, double innerFilletRadius, double precision);
	TopoDS_Wire BuildRoundedRectangle(double dimX, double dimY, double roundingRadius, const TopLoc_Location& location, double precision);
	TopoDS_Wire BuildIShape(double overallWidth, double overallDepth, double flangeThickness, double webThickness, double filletRadius, const TopLoc_Location& location, double precision, bool detailed);
	TopoDS_Wire BuildTShape(double flangeWidth, double depth, double flangeThickness, double webThickness, double flangeSlope, double webSlope, double flangeEdgeRadius, double filletRadius, double webEdgeRadius, const TopLoc_Location& location, double angleToRadians, bool detailed);
	TopoDS_Wire BuildTrapezium(double bottomDimX, double topDimX, double dimY, double topOffsetX, const TopLoc_Location& location);
	TopoDS_Wire BuildLShape(double depth, double width, double thickness, double legSlope, double edgeRadius, double filletRadius, const TopLoc_Location& location, double angleToRadians, bool detailed);
	TopoDS_Wire BuildUShape(double flangeWidth, double depth, double flangeThickness, double webThickness, double flangeSlope, double edgeRadius, double filletRadius, const TopLoc_Location& location, double radianFactor, bool detailed);
	TopoDS_Wire BuildZShape(double flangeWidth, double depth, double flangeThickness, double webThickness, double edgeRadius, double filletRadius, const TopLoc_Location& location, bool detailed);
	TopoDS_Face BuildMirrored(const TopoDS_Face& parentFace);
	TopoDS_Wire BuildAsymmetricIShape(double bottomFlangeWidth, double overallDepth, double webThickness, double bottomFlangeThickness, double bottomFlangeFilletRadius, double topFlangeWidth, double topFlangeThickness, double topFlangeFilletRadius, double bottomFlangeEdgeRadius, double bottomFlangeSlope, double topFlangeEdgeRadius, double topFlangeSlope, const TopLoc_Location& location, double precision, bool detailed);
	TopoDS_Wire BuildCShape(double width, double depth, double girth, double wallThickness, double internalFilletRadius, const TopLoc_Location& location, bool detailed);
};

