#pragma once

#include <Geom_Axis1Placement.hxx>
#include "../XbimHandle.h"

#include "XDirection.h"
#include "XPoint.h"
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XAxis1Placement : XbimHandle<Handle(Geom_Axis1Placement)>, IXAxis1Placement
			{
			public:
				XAxis1Placement(Handle(Geom_Axis1Placement) axis1) : XbimHandle(new Handle(Geom_Axis1Placement)(axis1)) { };				
				virtual property IXPoint^ Location { IXPoint^ get() { return gcnew XPoint(OccHandle()->Location()); }};
				virtual property IXDirection^ Direction { IXDirection^ get() { return gcnew XDirection(OccHandle()->Direction()); }};
			};
		}
	}
}