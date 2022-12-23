#pragma once
#include <Geom2d_AxisPlacement.hxx>
#include "../XbimHandle.h"
#include "../BRep/X2dPoint.h"
#include "../BRep/X2dDirection.h"
#include <gp_Ax22d.hxx>
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XAxisPlacement2d : XbimHandle<Handle(Geom2d_AxisPlacement)>, IXAxisPlacement2d
			{
			public:
				XAxisPlacement2d(Handle(Geom2d_AxisPlacement) axis1) : XbimHandle(new Handle(Geom2d_AxisPlacement)(axis1)) {};
				virtual property IXPoint^ Location {IXPoint^ get() { return gcnew X2dPoint(OccHandle()->Location()); }};
				virtual property IXDirection^ Direction {IXDirection^ get() { return gcnew X2dDirection(OccHandle()->Direction()); }};
				virtual property IXDirection^ YDirection {IXDirection^ get() { return gcnew X2dDirection(gp_Ax22d(OccHandle()->Ax2d()).YDirection()); }};
			};
		}
	}
}