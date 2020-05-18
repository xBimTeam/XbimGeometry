#pragma once
#include "XbimShape.h"
#include <TopoDS_Shell.hxx>

using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimShell : public XbimShape<TopoDS_Shell>, IXShell
			{
			public:
				XbimShell(const TopoDS_Shell& hShell) : XbimShape(new TopoDS_Shell(hShell)) {  };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Shell; } };
				virtual property IEnumerable<IXFace^>^ Faces {IEnumerable<IXFace^>^ get(); };
			};
		};


	}
}


