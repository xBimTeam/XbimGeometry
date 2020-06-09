#include "FaceFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <TopLoc_Datum3D.hxx>
#include <gp_Pln.hxx>
TopoDS_Face Xbim::Geometry::Factories::FaceFactory::BuildProfileDef(IIfcProfileDef^ profileDef)
{
	TopoDS_Wire wire = _wireFactory->BuildProfile(profileDef); //this will throw an exception if it fails
	//all profile defs are 2D with Z = 0,0,1
	gp_Pnt origin = gp::Origin();
	gp_Dir zDir = gp::DZ();
	gp_Pln surface(origin, zDir);
	TopoDS_Face face = Ptr()->BuildProfileDef(surface, wire);
	if (face.IsNull() || face.NbChildren() == 0)
		throw gcnew XbimGeometryFactoryException("Could not build face from profiel definition");
	return face;
}
