#pragma once
#include "XbimShape.h"
#include <TopoDS_Face.hxx>


using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimFace : public XbimShape<TopoDS_Face>, IXFace
			{
			public:
				XbimFace(const TopoDS_Face& hFace) :XbimShape(new TopoDS_Face(hFace)) { };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Face; } };
				virtual property double Tolerance { double get(); };
				virtual property IXWire^ OuterBound {IXWire^ get(); };
				virtual property IEnumerable<IXWire^>^ InnerBounds {IEnumerable<IXWire^>^ get(); };
				virtual property IXSurface^ Surface {IXSurface^ get(); };

			
			};

			
		}
	}
}


class NXbimFace
{
public:
	static TopoDS_ListOfShape InnerWires(const TopoDS_Face& face);
};


