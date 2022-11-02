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

			IEnumerable<IXWire^>^ XFace::InnerBounds::get()
			{
				TopoDS_ListOfShape innerWires = NXbimFace::InnerWires(OccFace());
				if (innerWires.Extent() == 0) return Enumerable::Empty<IXWire^>();
				List< IXWire^>^ inners = gcnew List<IXWire^>(innerWires.Extent());
				for (TopTools_ListIteratorOfListOfShape iter(innerWires); iter.More(); iter.Next())
				{
					inners->Add(gcnew XWire(TopoDS::Wire(iter.Value())));
				}
				return inners;
			};

			IXSurface^ XFace::Surface::get()
			{
				Handle(Geom_Surface) surface = BRep_Tool::Surface(OccFace());

				GeomAdaptor_Surface gs(surface);
				GeomAbs_SurfaceType surfaceType = gs.GetType();
				String^ surfaceTypeName = "";
				switch (surfaceType)
				{
				case GeomAbs_Plane:
					return gcnew XPlane(Handle(Geom_Plane)::DownCast(surface));
				case GeomAbs_Cylinder:
					return gcnew XCylindricalSurface(Handle(Geom_CylindricalSurface)::DownCast(surface));
				case GeomAbs_Cone:
					return gcnew XConicalSurface(Handle(Geom_ConicalSurface)::DownCast(surface));
				case GeomAbs_Sphere:
					return gcnew XbimSphericalSurface(Handle(Geom_SphericalSurface)::DownCast(surface));
				case GeomAbs_Torus:
					surfaceTypeName = "GeomAbs_Torus";
					break;
				case GeomAbs_BezierSurface:
					surfaceTypeName = "GeomAbs_BezierSurface";
					break;
				case GeomAbs_BSplineSurface:
					surfaceTypeName = "GeomAbs_BSplineSurface";
					break;
				case GeomAbs_SurfaceOfRevolution:
					surfaceTypeName = "GeomAbs_SurfaceOfRevolution";
					break;
				case GeomAbs_SurfaceOfExtrusion:
					surfaceTypeName = "GeomAbs_SurfaceOfExtrusion";
					break;
				case GeomAbs_OffsetSurface:
					surfaceTypeName = "GeomAbs_OffsetSurface";
					break;
				case GeomAbs_OtherSurface:
					surfaceTypeName = "GeomAbs_OtherSurface";
					break;
				default:
					surfaceTypeName = "Unknown_SurfaceType";
					break;
				}
				
				throw gcnew XbimGeometryDefinitionException(String::Format("Surface type not implemented: {0}", surfaceTypeName));
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

