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
			public ref class Xbim2dVector : IXVector
			{
			private:
				double x;
				double y;
				double magnitude;
			public:
				Xbim2dVector() : x(0.0), y(0.0), magnitude(1.0) {};
				/*Xbim2dVector(double x, double y) : x(x), y(y) {
					gp_Vec2d v(x, y);
					magnitude = v.Magnitude();
					v.Normalize();
					x = v.X();
					y = v.Y();
				};
				Xbim2dVector(double x, double y, double magnitide) : x(x), y(y), magnitude(magnitude) {
					gp_Vec2d v(x, y);
					v.Normalize();
					x = v.X();
					y = v.Y();
				};*/

				// Create a normalised unit vector in the direction of d with magnitude 1.0
				Xbim2dVector(const gp_Dir2d& d) : Xbim2dVector(gp_Vec2d(d), 1.0) { };
				// Create a normalised unit vector in the direction of d with magnitude 1.0
				Xbim2dVector(const gp_Vec2d& v) : Xbim2dVector(v, 1.0) { };
				//Create a normalised unit vector in the direction of v with magnitude length
				Xbim2dVector(const gp_Dir2d& d, double len) : Xbim2dVector(gp_Vec2d(d), len) {}
				//Create a normalised unit vector in the direction of v with magnitude length
				Xbim2dVector(const gp_Vec2d& v, double len)
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
			};
		}
	}
}