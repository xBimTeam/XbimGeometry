#pragma once

#include <Geom_Axis1Placement.hxx>
#include "../XbimHandle.h"

#include "../BRep/XbimDirection.h"
#include "../BRep/XbimPoint.h"
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XbimAxis1Placement : XbimHandle<Handle(Geom_Axis1Placement)>, IXAxis1Placement
			{
			public:
				XbimAxis1Placement(Handle(Geom_Axis1Placement) axis1) : XbimHandle(new Handle(Geom_Axis1Placement)(axis1)) { };				
				virtual property IXPoint^ Location { IXPoint^ get() { return gcnew XbimPoint(OccHandle()->Location()); }};
				virtual property IXDirection^ Direction { IXDirection^ get() { return gcnew XbimDirection(OccHandle()->Direction()); }};
			};
		}
	}
}