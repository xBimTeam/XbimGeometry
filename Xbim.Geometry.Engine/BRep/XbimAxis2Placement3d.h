#pragma once
#include <Geom_Axis2Placement.hxx>
#include <Geom_Axis1Placement.hxx>
#include "../XbimHandle.h"
#include "../BRep/XbimAxis1Placement.h"
#include "../BRep/XbimDirection.h"

using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XbimAxis2Placement3d : XbimHandle<Handle(Geom_Axis2Placement)>, IXAxis2Placement3d
			{
			public:
				XbimAxis2Placement3d(Handle(Geom_Axis2Placement) axis2) : XbimHandle(new Handle(Geom_Axis2Placement)(axis2)) { };
				virtual property IXAxis1Placement^ Axis { IXAxis1Placement^ get() { return gcnew XbimAxis1Placement(Handle(Geom_Axis1Placement)(new Geom_Axis1Placement(OccHandle()->Axis()))); }};
				virtual property IXDirection^ XDirection { IXDirection^ get() { return gcnew XbimDirection(OccHandle()->XDirection()); }};
				virtual property IXDirection^ YDirection { IXDirection^ get() { return gcnew XbimDirection(OccHandle()->YDirection()); }};
			};
		}
	}
}

