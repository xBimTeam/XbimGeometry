#pragma once
#include <Geom_Axis2Placement.hxx>
#include <Geom_Axis1Placement.hxx>
#include "../XbimHandle.h"
#include "XAxis1Placement.h"
#include "XDirection.h"

using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XAxis2Placement3d : XbimHandle<Handle(Geom_Axis2Placement)>, IXAxis2Placement3d
			{
			public:
				XAxis2Placement3d(Handle(Geom_Axis2Placement) axis2) : XbimHandle(new Handle(Geom_Axis2Placement)(axis2)) { };
				virtual property IXAxis1Placement^ Axis { IXAxis1Placement^ get() { return gcnew XAxis1Placement(Handle(Geom_Axis1Placement)(new Geom_Axis1Placement(OccHandle()->Axis()))); }};
				virtual property IXDirection^ XDirection { IXDirection^ get() { return gcnew Xbim::Geometry::BRep::XDirection(OccHandle()->XDirection()); }};
				virtual property IXDirection^ YDirection { IXDirection^ get() { return gcnew Xbim::Geometry::BRep::XDirection(OccHandle()->YDirection()); }};
			};
		}
	}
}

