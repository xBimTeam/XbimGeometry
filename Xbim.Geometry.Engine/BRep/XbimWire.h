#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Wire.hxx>


using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimWire : XbimHandle<TopoDS_Wire>, IXWire
			{
			public:
				XbimWire(const TopoDS_Wire& hWire) : XbimHandle(new TopoDS_Wire(hWire)) {};
				virtual property XShapeType ShapeType { XShapeType get() { return XShapeType::Wire; } };
				virtual property IEnumerable<IXEdge^>^ EdgeLoop {IEnumerable<IXEdge^>^ get(); };
			};
		}
	}
}

