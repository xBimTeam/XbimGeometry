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
			public ref struct XPoint : public IXPoint
			{
			private:
				double _x;
				double _y;
				double _z;
			public:
				XPoint(const gp_XYZ& pnt) :_x(pnt.X()), _y(pnt.Y()), _z(pnt.Z()) {};
				XPoint(gp_XYZ* pPnt) :_x(pPnt->X()), _y(pPnt->Y()), _z(pPnt->Z()) {};
				XPoint(const gp_Pnt& pnt) :_x(pnt.X()), _y(pnt.Y()), _z(pnt.Z()) {};
				XPoint(gp_Pnt* pPnt) :_x(pPnt->X()), _y(pPnt->Y()), _z(pPnt->Z()) {};
				XPoint(const gp_Pnt2d& pnt2d) : _x(pnt2d.X()), _y(pnt2d.Y()), _z(double::NaN) {};
				XPoint(gp_Pnt2d* pPnt2d) : _x(pPnt2d->X()), _y(pPnt2d->Y()), _z(double::NaN) {};
				XPoint(const gp_XY& pnt2d) : _x(pnt2d.X()), _y(pnt2d.Y()), _z(double::NaN) {};
				XPoint(gp_XY* pPnt2d) : _x(pPnt2d->X()), _y(pPnt2d->Y()), _z(double::NaN) {};
				XPoint(double x, double y, double z) :_x(x), _y(y), _z(z) {};
				XPoint(double x, double y) :_x(x), _y(y), _z(double::NaN) {};
				virtual property bool Is3d { bool get() { return !double::IsNaN(_z); }; };
				virtual property double X { double get() { return _x; }; void set(double v) { _x = v; }};
				virtual property double Y { double get() { return _y; }; void set(double v) { _y = v; }};
				virtual property double Z { double get() { return _z; }; void set(double v) { _z = v; }};
			};
		}
	}
}



