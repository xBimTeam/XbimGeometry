#pragma once
#include "XbimShape.h"
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
			public ref class XbimSolid : public XbimShape<TopoDS_Solid>, IXSolid
			{
			public:
				XbimSolid(const TopoDS_Solid& solid) : XbimShape<TopoDS_Solid>(new TopoDS_Solid(solid)) {  };
				virtual property XShapeType ShapeType { XShapeType get() override  { return XShapeType::Solid; }};
				
				virtual property IEnumerable<IXShell^>^ Shells {IEnumerable<IXShell^>^ get(); };
				virtual double Volume();
			};
		}
	}
}

