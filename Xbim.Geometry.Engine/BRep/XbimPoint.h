#pragma once
#include "../XbimHandle.h"
#include <gp_Pnt.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref struct XbimPoint : XbimHandle<gp_Pnt>, public IXPoint
			{

			public:
				XbimPoint(const gp_Pnt& pnt) : XbimHandle(new gp_Pnt(pnt)) {};
				XbimPoint(gp_Pnt* pPnt) : XbimHandle(pPnt) {};
				XbimPoint(const gp_Pnt2d& pnt2d) : XbimHandle(new gp_Pnt(pnt2d.X(), pnt2d.Y(), .0)) {};
				XbimPoint(gp_Pnt2d* pPnt2d) : XbimHandle(new gp_Pnt(pPnt2d->X(), pPnt2d->Y(), .0)) {};
				XbimPoint(double x, double y, double z) : XbimHandle(new gp_Pnt(x, y, z)) {};
				virtual property bool Is3d { bool get() { return true; }; };
				virtual property double X { double get() { return OccHandle().X(); }; void set(double v) { OccHandle().SetX(v); }};
				virtual property double Y { double get() { return  OccHandle().Y(); }; void set(double v) { OccHandle().SetY(v); }};
				virtual property double Z { double get() { return  OccHandle().Z(); }; void set(double v) { OccHandle().SetZ(v); }};
			};
		}
	}
}



