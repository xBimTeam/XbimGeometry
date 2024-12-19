#pragma once
#include "XShape.h"
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include "XSurface.h"
using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;
#define OccFace() TopoDS::Face(*(this->Ptr()))
#define TOPO_FACE(face) TopoDS::Face(static_cast<XFace^>(face)->GetTopoShape())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XFace : public XShape, IXFace, IXBoundedSurface
			{
			public:
				XFace(const TopoDS_Face& hFace) :XShape(new TopoDS_Face(hFace)) { };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Face; } };
				virtual property double Tolerance { double get(); };
				virtual property IXWire^ OuterBound {IXWire^ get(); };
				virtual property array<IXWire^>^ InnerBounds {array<IXWire^>^ get(); };
				virtual property IXSurface^ Surface {IXSurface^ get(); };
				/// <summary>
				/// Area in default model units
				/// </summary>
				virtual property double Area {double get(); }
				virtual property XSurfaceType SurfaceType {virtual XSurfaceType get() { return XSurfaceType::IfcCurveBoundedSurface; }; };
				virtual property bool IsUPeriodic {bool get() { return Surface->IsUPeriodic; }}
				virtual property bool IsVPeriodic {bool get() { return Surface->IsVPeriodic; }}
				
			};

			
		}
	}
}


class NXbimFace
{
public:
	static TopoDS_ListOfShape InnerWires(const TopoDS_Face& face);
};


