#pragma once
#include "XbimShape.h"
#include <TopoDS_Wire.hxx>


using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimWire : public XbimShape<TopoDS_Wire>, IXWire
			{
			public:
				XbimWire(const TopoDS_Wire& hWire) : XbimShape(new TopoDS_Wire(hWire)) {};
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Wire; } };
				virtual property IEnumerable<IXEdge^>^ EdgeLoop {IEnumerable<IXEdge^>^ get(); };
			};
		}
	}
}

