#include "NSurfaceFactory.h"
#include <BRep_Tool.hxx>
#include <GeomPlate_BuildAveragePlane.hxx>
#include <ShapeFix_Edge.hxx>


Handle(Geom_Plane) NSurfaceFactory::BuildPlane(double originX, double originY, double originZ, double normalX, double normalY, double normalZ)
{
	try
	{
		return new Geom_Plane(gp_Pnt(originX, originY, originZ), gp_Dir(normalX, normalY, normalZ)); //normal may throw an exception
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_Plane)();
	}
}

Handle(Geom_Plane) NSurfaceFactory::BuildPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& refDir)
{
	try
	{
		gp_Ax3 ax3(origin,normal,refDir);
		//this will throw an exception if the direction is 0
		return new Geom_Plane(ax3);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_Plane)();
	}
}

Handle(Geom_Plane) NSurfaceFactory::FindBestFitPlane(const Handle(TColgp_HArray1OfPnt)& points, const double tolerance)
{
	GeomPlate_BuildAveragePlane averagePlaneBuilder(points, points->Size(), tolerance, 2, 2);
	if (averagePlaneBuilder.IsPlane())
		return averagePlaneBuilder.Plane();
	else 
		return Handle(Geom_Plane)();
}

Handle(Geom_CylindricalSurface) NSurfaceFactory::BuildCylindricalSurface(double radius)
{
	try
	{
		return new Geom_CylindricalSurface(gp::XOY(), radius);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_CylindricalSurface)();
	}
}

Handle(Geom_SurfaceOfLinearExtrusion) NSurfaceFactory::BuildSurfaceOfLinearExtrusion(Handle(Geom_Curve) sweptCurve, const gp_Vec& sweepDirection)
{

	try
	{
		return new Geom_SurfaceOfLinearExtrusion(sweptCurve, sweepDirection);
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_SurfaceOfLinearExtrusion)();
	}
}

/// <summary>
/// This creates a surface from the curve within the edge and transforms the surface to the location defined in the edge
/// </summary>
/// <param name="sweptEdge"></param>
/// <param name="sweepDirection"></param>
/// <returns></returns>
Handle(Geom_SurfaceOfLinearExtrusion) NSurfaceFactory::BuildSurfaceOfLinearExtrusion(const TopoDS_Edge& sweptEdge, const gp_Vec& sweepDirection, bool hasRevitBSplineIssue)
{
	try
	{
		TopLoc_Location loc;
		double start = 0, end = 0;
		Handle(Geom_Curve) sweptCurve = BRep_Tool::Curve(sweptEdge, loc, start, end);
		//to get an accurate ruled surface we should use the trim parameters to define the bounds
		//however, when the surface is used in aspects such as IfcSurfaceCurveSweptAreaSolid then some BIM tools redefine the bounds to be larger than the ruled surface
		//this will throw an error, should we need rigidly bounded surafaces it is suggested these are built as Faces with appropriate Edges
		//To create a ruled surface use the line below and pass the TrimmedCurve to the Geom_SurfaceOfLinearExtrusion ctor
		//Handle(Geom_TrimmedCurve) trimmedCurve = new Geom_TrimmedCurve(sweptCurve, start, end);
		Handle(Geom_SurfaceOfLinearExtrusion) surface = new Geom_SurfaceOfLinearExtrusion(sweptCurve, sweepDirection);
		if(!hasRevitBSplineIssue && !loc.IsIdentity())
			surface->Transform(loc.Transformation());
		return surface;
	}
	catch (const Standard_Failure& sf)
	{
		LogStandardFailure(sf);
		return Handle(Geom_SurfaceOfLinearExtrusion)();
	}
}


TopoDS_Shape NSurfaceFactory::CreateSurfaceShape
(const std::vector<std::vector<TaggedPoint>>& allPoints, const std::vector<TopLoc_Location>& locations, const std::vector<std::string>& tags)
{
	std::vector<TopoDS_Shape> surfaces;
	for (size_t i = 0; i < tags.size() - 1; ++i)
	{
		std::string currenTag = tags[i];
		std::string nextTag = tags[i + 1];
		TopoDS_Wire wire1 = CreateWireFromTag(allPoints, locations, currenTag);
		TopoDS_Wire wire2 = CreateWireFromTag(allPoints, locations, nextTag);
		TopoDS_Shape surface = CreateSurfaceShapeFromWires(wire1, wire2);
		surfaces.push_back(surface);
	}

	BRepBuilderAPI_Sewing sewingTool;
	for (const auto& shape : surfaces)
	{
		sewingTool.Add(shape);
	}

	sewingTool.Perform();
	return sewingTool.SewedShape();
}


TopoDS_Shape NSurfaceFactory::CreateSurfaceShapeFromWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2)
{

	std::vector<TopoDS_Edge> edges1;
	std::vector<TopoDS_Edge> edges2;
	BRepBuilderAPI_Sewing sewingTool;

	for (TopExp_Explorer explorer(wire1, TopAbs_EDGE); explorer.More(); explorer.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
		edges1.push_back(edge);
	}

	for (TopExp_Explorer explorer(wire2, TopAbs_EDGE); explorer.More(); explorer.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(explorer.Current());
		edges2.push_back(edge);
	}

	for (size_t i = 0; i < edges1.size(); ++i) {
		TopoDS_Shape ruledSurface = BRepFill::Face(edges1[i], edges2[i]);
		sewingTool.Add(ruledSurface);
	}

	sewingTool.Perform();
	return sewingTool.SewedShape();

}


TopoDS_Wire NSurfaceFactory::CreateWireFromTag
(const std::vector<std::vector<TaggedPoint>>& allPoints, const std::vector<TopLoc_Location>& locations, const std::string& tag)
{
	BRepBuilderAPI_MakePolygon polygon;
	size_t i = 0;

	for (const auto& vec : allPoints) {
		const TopLoc_Location& location = locations[i];
		for (const auto& taggedPoint : vec) {
			if (taggedPoint.tag == tag) {
				const gp_Pnt& transformed = taggedPoint.point.Transformed(location.Transformation());
				polygon.Add(transformed);
			}
		}
		i++;
	}

	return polygon.Wire();
}

void NSurfaceFactory::FixInvalidEdges(const TopoDS_Face& face) {
	ShapeFix_Edge sfe;
	for (TopExp_Explorer exp(face, TopAbs_EDGE); exp.More(); exp.Next())
	{
		sfe.FixAddPCurve(TopoDS::Edge(exp.Current()), face, Standard_False);
	}
}