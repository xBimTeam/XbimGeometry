#pragma once
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>
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
				double x = double::NaN;
				double y = double::NaN;
				double z = double::NaN;

			public:
				XDirection() {};
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
					catch (const Standard_Failure& )
					{
						x = double::NaN;
						y = double::NaN;
						z = double::NaN;
					}
				};
				XDirection(double x, double y) : z(double::NaN)
				{
					try
					{
						gp_Vec2d v(x, y);
						v.Normalize();
						this->x = v.X();
						this->y = v.Y();
					}
					catch (const Standard_Failure& )
					{
						
						x = double::NaN;
						y = double::NaN;
					}
				};
				// Create a normalised unit vector with direction of d 
				XDirection(const gp_Dir& d) : x(d.X()), y(d.Y()), z(d.Z()) { };
				XDirection(const gp_Dir2d& d) : x(d.X()), y(d.Y()), z(double::NaN) { };
				virtual property bool IsNull { bool get() { return double::IsNaN(x) || double::IsNaN(y); }};

				virtual property bool Is3d { bool get() { return !double::IsNaN(z); }; };
				virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
				virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
				virtual property double Z { double get() { return z; }; void set(double v) { z = v; }};
			};

		}
	}
}