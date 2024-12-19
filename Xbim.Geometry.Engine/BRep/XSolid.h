#pragma once
#include "XShape.h"
#include <TopoDS_Solid.hxx>
#include <TopoDS.hxx>

using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;
#define OccSolid() TopoDS::Solid(*(this->Ptr()))
#define TOPO_SOLID(solid) TopoDS::Solid(static_cast<XbimSolid^>(solid)->GetTopoShape())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XSolid : public XShape, IXSolid
			{
			public:
				XSolid(const TopoDS_Solid& solid) : XShape(new TopoDS_Solid(solid)) {  };
				virtual property XShapeType ShapeType { XShapeType get() override  { return XShapeType::Solid; }};
				
				virtual property array<IXShell^>^ Shells {array<IXShell^>^ get(); };
				virtual property double Volume {double get(); }

				
			};
		}
	}
}

