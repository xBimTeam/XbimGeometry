#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Solid.hxx>
using namespace System;
using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimSolid : XbimHandle<TopoDS_Solid>, IXSolid
			{
			public:
				XbimSolid(const TopoDS_Solid& solid) : XbimHandle(new TopoDS_Solid(solid)) {  };
				virtual property XShapeType ShapeType { XShapeType get() { return XShapeType::Solid; } };
				virtual String^ BRepDefinition();
				virtual property IEnumerable<IXShell^>^ Shells {IEnumerable<IXShell^>^ get(); };

			};
		}
	}
}

