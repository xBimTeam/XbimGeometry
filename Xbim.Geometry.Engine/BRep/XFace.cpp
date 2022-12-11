#include "XFace.h"
#include "XWire.h"
#include "XPlane.h"
#include "XConicalSurface.h"
#include "XCylindricalSurface.h"
#include "XSphericalSurface.h"
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <TopoDS_Wire.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopoDS_ListOfShape.hxx>
#include <Geom_Plane.hxx>
#include <Geom_ConicalSurface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include "../Exceptions/XbimGeometryDefinitionException.h"
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
#include <GeomLProp_SLProps.hxx>
#include <ShapeAnalysis_Surface.hxx>
using namespace System;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			double XFace::Area::get()
			{
				GProp_GProps gProps;
				BRepGProp::SurfaceProperties(OccFace(), gProps);
				return gProps.Mass();
			}

			double XFace::Tolerance::get()
			{
				return BRep_Tool::Tolerance(OccFace());
			};

			IXWire^ XFace::OuterBound::get()
			{
				TopoDS_Wire outerWire = BRepTools::OuterWire(OccFace());
				return gcnew XWire(outerWire);
			};

			array<IXWire^>^ XFace::InnerBounds::get()
			{
				TopExp_Explorer faceEx(OccFace(), TopAbs_FACE);
				TopoDS_ListOfShape shapes = NXbimFace::InnerWires(OccFace());
				array<IXWire^>^ managedShapes = gcnew  array<IXWire^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = gcnew XWire(TopoDS::Wire(shape));
				return managedShapes;

			};

			IXSurface^ XFace::Surface::get()
			{
				Handle(Geom_Surface) surface = BRep_Tool::Surface(OccFace());
				return XSurface::GeomToXSurface(surface);
			};

			
		}
	}
}

#pragma managed(push, off)

TopoDS_ListOfShape NXbimFace::InnerWires(const TopoDS_Face& face)
{
	TopoDS_Wire outerWireCandidate;
	TopExp_Explorer expw(face, TopAbs_WIRE);
	TopoDS_ListOfShape innerWires;
	if (expw.More()) //if we have more then one get the outer and inners
	{
		outerWireCandidate = TopoDS::Wire(expw.Current());
		expw.Next();
		if (expw.More()) {
			Standard_Real UMin, UMax, VMin, VMax;
			Standard_Real umin, umax, vmin, vmax;
			BRepTools::UVBounds(face, outerWireCandidate, UMin, UMax, VMin, VMax);
			while (expw.More()) {
				const TopoDS_Wire& W = TopoDS::Wire(expw.Current());
				BRepTools::UVBounds(face, W, umin, umax, vmin, vmax);
				if ((umin <= UMin) &&
					(umax >= UMax) &&
					(vmin <= VMin) &&
					(vmax >= VMax)) //we have a new outer wire
				{
					innerWires.Append(outerWireCandidate); //its no longer an outer  candidate
					outerWireCandidate = W;
					UMin = umin;
					UMax = umax;
					VMin = vmin;
					VMax = vmax;
				}
				else //this is an inner
				{
					innerWires.Append(W);
				}
				expw.Next();
			}
		}
	}
	return innerWires;

}
#pragma managed(pop)

