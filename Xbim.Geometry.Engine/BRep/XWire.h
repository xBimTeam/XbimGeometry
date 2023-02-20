#pragma once
#include "XShape.h"
#include <TopoDS_Wire.hxx>
#include <TopoDS.hxx>

#define OccWire() TopoDS::Wire(*(this->Ptr()))
#define TOPO_WIRE(wire) TopoDS::Wire(static_cast<XWire^>(wire)->GetTopoShape())

using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XWire : public XShape, IXWire
			{
			public:
				XWire(const TopoDS_Wire& hWire) : XShape(new TopoDS_Wire(hWire)) {};
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Wire; } };
				virtual property array<IXEdge^>^ EdgeLoop {array<IXEdge^>^ get(); };
				virtual property double Length {double get(); };
				virtual property double ContourArea {double get(); };

				
			};
		}
	}
}

