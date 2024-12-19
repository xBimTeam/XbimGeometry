#pragma once
#include "NFactoryBase.h"
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <vector>
#include "../../BRep/TaggedPoint.h"
#include <TopExp_Explorer.hxx>
#include <BRepFill.hxx>
#include <TopoDS.hxx>

using namespace Xbim::Geometry::BRep;

class NSurfaceFactory : public NFactoryBase
{

public:
	Handle(Geom_Plane) BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ);
	Handle(Geom_Plane) BuildPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& refDir);
	Handle(Geom_Plane) FindBestFitPlane(const Handle(TColgp_HArray1OfPnt)& points, const double tolerance);
	Handle(Geom_CylindricalSurface) BuildCylindricalSurface(double radius);
	Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(Handle(Geom_Curve) sweptCurve, const gp_Vec& sweepDirection);
	Handle(Geom_SurfaceOfLinearExtrusion) BuildSurfaceOfLinearExtrusion(const TopoDS_Edge& sweptEdge, const gp_Vec& sweepDirection, bool hasRevitBSplineIssue);
	TopoDS_Shape CreateSurfaceShape
		(const std::vector<std::vector<TaggedPoint>> &allPoints, const std::vector<TopLoc_Location>& locations, const std::vector<std::string>& tags);


private:
	TopoDS_Wire CreateWireFromTag
	(const std::vector<std::vector<TaggedPoint>>& allPoints, const std::vector<TopLoc_Location>& locations, const std::string& tag);
	TopoDS_Shape CreateSurfaceShapeFromWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);
};

