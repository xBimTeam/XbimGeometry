#pragma once
#include <Geom2d_AxisPlacement.hxx>
#include "../XbimHandle.h"
#include "../BRep/Xbim2dPoint.h"
#include "../BRep/Xbim2dDirection.h"
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{

			public ref class XbimAxisPlacement2d : XbimHandle<Handle(Geom2d_AxisPlacement)>, IXAxis2Placement2d
			{
			public:
				XbimAxisPlacement2d(Handle(Geom2d_AxisPlacement) axis1) : XbimHandle(new Handle(Geom2d_AxisPlacement)(axis1)) {};
				virtual property IXPoint^ Location {IXPoint^ get() { return gcnew Xbim2dPoint(OccHandle()->Location()); }};
				virtual property IXDirection^ Direction {IXDirection^ get() { return gcnew Xbim2dDirection(OccHandle()->Direction()); }};

			};
		}
	}

