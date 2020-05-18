#pragma once
#include "XbimShape.h"
#include <TopoDS_Compound.hxx>

using namespace System;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimCompound : public XbimShape<TopoDS_Compound>, IXCompound
			{
			public:
				XbimCompound(const TopoDS_Compound& hCompound) : XbimShape(new TopoDS_Compound(hCompound)) { };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Compound; } };
				virtual String^ BRepDefinition();
				virtual property bool IsSolidsOnly {bool get(); };
				virtual property bool IsShellsOnly {bool get(); };
				virtual property bool IsFacesOnly {bool get(); };
				virtual property bool HasSolids {bool get(); };
				virtual property bool HasShells {bool get(); };
				virtual property bool HasFaces {bool get(); };
				virtual property IEnumerable<IXSolid^>^ Solids { IEnumerable<IXSolid^>^ get(); };
				virtual property IEnumerable<IXShell^>^ Shells { IEnumerable<IXShell^>^ get(); };
				virtual property IEnumerable<IXFace^>^ Faces { IEnumerable<IXFace^>^ get(); };
			};
		}
	}
}

