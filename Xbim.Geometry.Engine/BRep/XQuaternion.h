#pragma once

#include <gp_Quaternion.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref struct XQuarternion : public IXQuaternion
			{
			private:
				double _x;
				double _y;
				double _z;
				double _w;
			public:
				XQuarternion(const gp_Quaternion& qt) :_x(qt.X()), _y(qt.Y()), _z(qt.Z()), _w(qt.W()) {};
				XQuarternion(double x, double y, double z, double w) :_x(x), _y(y), _z(z), _w(w) {};

				virtual property bool Is3d { bool get() { return true; }; };
				virtual property double X { double get() { return _x; }; void set(double v) { _x = v; }};
				virtual property double Y { double get() { return _y; }; void set(double v) { _y = v; }};
				virtual property double Z { double get() { return _z; }; void set(double v) { _z = v; }};
				virtual property double W { double get() { return _w; }; void set(double v) { _w = v; }};
			};


		}
	}
}
