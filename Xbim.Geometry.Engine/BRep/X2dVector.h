#pragma once
#include <gp_Pnt2d.hxx>
#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class X2dVector : IXVector
			{
			private:
				double x;
				double y;
				double magnitude;
			public:
				X2dVector() : x(0.0), y(0.0), magnitude(1.0) {};				
				// Create a normalised unit vector in the direction of d with magnitude 1.0
				X2dVector(const gp_Dir2d& d) : X2dVector(gp_Vec2d(d), 1.0) { };
				// Create a normalised unit vector in the direction of d with magnitude 1.0
				X2dVector(const gp_Vec2d& v) : X2dVector(v, 1.0) { };
				//Create a normalised unit vector in the direction of v with magnitude length
				X2dVector(const gp_Dir2d& d, double len) : X2dVector(gp_Vec2d(d), len) {}
				//Create a normalised unit vector in the direction of v with magnitude length
				X2dVector(const gp_Vec2d& v, double len)
				{
					magnitude = len;
					gp_Vec2d nv = v.Normalized();
					x = nv.X();
					y = nv.Y();

				};
				virtual property bool Is3d { bool get() { return false; }; };
				virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
				virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
				virtual property double Z {
					double get() { throw gcnew XbimGeometryFactoryException("2d vectors do not support Z values"); };
					void set(double v) { throw gcnew XbimGeometryFactoryException("2d vectors do not support Z values"); }
				};
				virtual property double Magnitude { double get() { return magnitude; }; void set(double v) { magnitude = v; }};
				virtual property bool IsNull { bool get() { return ((x * x) + (y * y)) <= 0; }; };
			};
		}
	}
}