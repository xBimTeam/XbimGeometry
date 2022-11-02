#pragma once
#include <gp_Vec.hxx>
#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			//Represents a unit vector
			public ref struct XDirection : public IXDirection
			{

			private:
				double x;
				double y;
				double z;

			public:
				XDirection() : x(0), y(0), z(1) {};
				//Create a normalised unit vector in the direction of x, y, z 
				XDirection(double x, double y, double z)
				{
					try
					{
						gp_Vec v(x, y, z);
						v.Normalize();
						this->x = v.X();
						this->y = v.Y();
						this->z = v.Z();
					}
					catch (...)
					{
						throw gcnew XbimGeometryFactoryException("Invalid unit vector definition");
					}

				};
				// Create a normalised unit vector with direction of d 
				XDirection(const gp_Dir& d) : x(d.X()), y(d.Y()), z(d.Z()) { };
				virtual property bool IsNull { bool get() { return ((x * x) + (y * y) + (z * z)) <= 0; }; };
				virtual property bool Is3d { bool get() { return true; }; };
				virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
				virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
				virtual property double Z { double get() { return z; }; void set(double v) { z = v; }};
			};

		}
	}
}