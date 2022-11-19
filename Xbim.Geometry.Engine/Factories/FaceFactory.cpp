#include "FaceFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <TopLoc_Datum3D.hxx>
#include <gp_Pln.hxx>
#include "../BRep/XSurface.h"
#include "../BRep/XPlane.h"
#include "../BRep/XFace.h"
using namespace System;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Linq;
TopoDS_Face Xbim::Geometry::Factories::FaceFactory::BuildProfileDef(IIfcProfileDef^ profileDef)
{
	TopoDS_Wire wire = _modelService->GetWireFactory()->BuildProfile(profileDef); //this will throw an exception if it fails
	//all profile defs are 2D with Z = 0,0,1
	gp_Pnt origin = gp::Origin();
	gp_Dir zDir = gp::DZ();
	gp_Pln surface(origin, zDir);
	TopoDS_Face face = Ptr()->BuildProfileDef(surface, wire);
	if (face.IsNull() || face.NbChildren() == 0)
		throw gcnew XbimGeometryFactoryException("Could not build face from profile definition");
	return face;
}

gp_Vec Xbim::Geometry::Factories::FaceFactory::Normal(const TopoDS_Face& face)
{
	return Ptr()->Normal(face);
}

TopoDS_Face Xbim::Geometry::Factories::FaceFactory::BuildPlanarFace(IXPlane^ planeDef)
{
	gp_Dir xDir(planeDef->RefDirection->X, planeDef->RefDirection->Y, planeDef->RefDirection->Is3d ? planeDef->RefDirection->Z : 0);
	gp_Dir zDir(planeDef->Axis->X, planeDef->Axis->Y, planeDef->Axis->Is3d ? planeDef->Axis->Z : 0);
	gp_Pnt loc(planeDef->Location->X, planeDef->Location->Y, planeDef->Location->Is3d ? planeDef->Location->Z : 0);
	gp_Ax3 ax3(loc, zDir, xDir);
	gp_Pln pln(ax3);
	BRepBuilderAPI_MakeFace  builder(pln);
	return builder.Face();
}

IXFace^ Xbim::Geometry::Factories::FaceFactory::BuildFace(IXSurface^ surface, array<IXWire^>^ wires)
{
	XSurface^ xbimSurface = dynamic_cast<XSurface^>(surface);
	if (wires->Length < 1)
		throw gcnew ArgumentOutOfRangeException("Wires array must contain at least one wire");
	if(xbimSurface == nullptr)
		throw gcnew InvalidCastException("Invalid surface object not created by Xbim");
	//find the largest wire, this will definitely be the outer bound
	double maxArea = 0;
	IXWire^ outerWire;
	for each (IXWire^ wire in wires)
	{
		double absArea = std::abs(wire->Area);
		if (absArea > maxArea)
		{
			maxArea = absArea;
			outerWire = wire;
		}
	}
	
	const TopoDS_Wire& topoWire = TopoDS::Wire(static_cast<XWire^>(outerWire)->GetTopoShape());
	BRepBuilderAPI_MakeFace  builder(xbimSurface->Ref(), topoWire);

	for each (IXWire ^ wire in wires)
	{
		if (wire != outerWire)
		{
			const TopoDS_Wire& topoInnerWire = TopoDS::Wire(static_cast<XWire^>(wire)->GetTopoShape());		
			builder.Add(topoInnerWire);
		}
	}
	TopoDS_Face result = builder.Face();
	result.Closed(outerWire->IsClosed);
	
	return gcnew XFace(result);
}
