#pragma once
#include "NFactoryBase.h"

#include <TopoDS_Solid.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <Geom_Curve.hxx>
#include <TopLoc_Location.hxx>
#include <Geom_Surface.hxx>
#include <vector>


class NSolidFactory : public NFactoryBase
{
	public:
		TopoDS_Solid BuildBlock(gp_Ax2 ax2, double xLength, double yLength, double zLength);
		TopoDS_Solid BuildRectangularPyramid(gp_Ax2 ax2, double xLength, double yLength, double height);
		TopoDS_Solid BuildRightCircularCone(gp_Ax2 ax2, double radius, double height);
		TopoDS_Solid BuildRightCylinder(gp_Ax2 ax2, double radius, double height);
		TopoDS_Solid BuildSphere(gp_Ax2 ax2, double radius);
		TopoDS_Solid BuildSweptDiskSolid(const Handle(Geom_Curve) directrixCurve, double radius, double innerRadius);
		TopoDS_Solid BuildSweptDiskSolid(const TopoDS_Wire& directrix, double radius, double innerRadius);
		TopoDS_Solid BuildSurfaceCurveSweptAreaSolid(const TopoDS_Face& sweptArea, const Handle(Geom_Surface)& refSurface, const TopoDS_Wire& directrixWire, bool isPlanarReferenceSurface, double precision);
		TopoDS_Solid BuildExtrudedAreaSolid(const TopoDS_Face& face, gp_Dir extrudeDirection, double depth, const TopLoc_Location& location);



		TopoDS_Solid MakeSolid(const TopoDS_Shell& faceSet);
		/// <summary>
		/// Casts a shape to a solid, any error are trapped and logged, empty solid is returned if failure
		/// </summary>
		/// <param name="shape"></param>
		/// <returns></returns>
		TopoDS_Solid CastToSolid(const TopoDS_Shape& shape);
		TopoDS_Solid MakeSweptSolid(const TopoDS_Face& face, const gp_Vec& direction);
		TopoDS_Shape BuildExtrudedAreaSolidTapered(const TopoDS_Face& sweptArea, const TopoDS_Face& endSweptArea, const gp_Dir& extrudeDirection, double depth, const TopLoc_Location& location, double precision);
		bool TryUpgrade(const TopoDS_Solid& solid, TopoDS_Shape& shape);
		TopoDS_Solid BuildSectionedSolid(const opencascade::handle<Geom_Curve>& spine, const std::vector<TopoDS_Wire>& sections);
		TopoDS_Solid BuildSectionedSolid(const opencascade::handle<Geom_Curve>& spine, const std::vector<TopoDS_Face>& sections);
		TopoDS_Solid BuildExtrudedAreaSolid(const opencascade::handle<Geom_Curve>& spine, const std::vector<TopoDS_Wire>& sections, double precision);

};

