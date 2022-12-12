#pragma once
#include "NFactoryBase.h"

#include <TopoDS_Solid.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <Geom_Curve.hxx>
class NSolidFactory : public NFactoryBase
{


public:

	TopoDS_Solid BuildBlock(gp_Ax2 ax2, double xLength, double yLength, double zLength);
	TopoDS_Solid BuildRectangularPyramid(gp_Ax2 ax2, double xLength, double yLength, double height);
	TopoDS_Solid BuildRightCircularCone(gp_Ax2 ax2, double radius, double height);
	TopoDS_Solid BuildRightCylinder(gp_Ax2 ax2, double radius, double height);
	TopoDS_Solid BuildSphere(gp_Ax2 ax2, double radius);
	TopoDS_Solid BuildSweptDiskSolid(const Handle(Geom_Curve) directrixCurve, double radius, double innerRadius, BRepBuilderAPI_TransitionMode transitionMode);
	TopoDS_Solid BuildSweptDiskSolid(const TopoDS_Wire& directrix, double radius, double innerRadius, BRepBuilderAPI_TransitionMode transitionMode);
	TopoDS_Solid BuildExtrudedAreaSolid(const TopoDS_Face& face, gp_Dir extrudeDirection, double depth);

	TopoDS_Solid MakeSolid(const TopoDS_Shell& faceSet);
	bool TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape);


};

