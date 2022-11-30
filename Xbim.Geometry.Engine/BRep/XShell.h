#pragma once
#include "XShape.h"
#include <TopoDS_Shell.hxx>
#include <TopoDS.hxx>

using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;
#define OccShell() TopoDS::Shell(*(this->Ptr()))
#define TOPO_SHELL(shell) TopoDS::Shell(static_cast<XbimShell^>(shell)->GetTopoShape())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XShell :  XShape, IXShell
			{
			public:
				XShell(const TopoDS_Shell& hShell) : XShape(new TopoDS_Shell(hShell)) {  };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Shell; } };
				virtual property array<IXFace^>^ Faces {array<IXFace^>^ get(); };
				virtual property double SurfaceArea {double get(); }

				
			};
		};


	}
}


