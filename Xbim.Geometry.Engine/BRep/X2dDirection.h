#pragma once
#include <gp_Dir2d.hxx>
#include "../Exceptions/XbimGeometryFactoryException.h"
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class X2dDirection : IXDirection
			{
			private:
				double x;
				double y;
				
			public:
				X2dDirection() : x(1.0), y(0.0) {};
				// Create a normalised unit vector in the direction of d with magnitude 1.0				
				X2dDirection(const gp_Dir2d& d)
				{
					x = d.X();
					y = d.Y();
				};
				virtual property bool Is3d { bool get() { return false; }; };
				virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
				virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
				virtual property double Z {
					double get() { throw gcnew XbimGeometryFactoryException("2d directions do not support Z values"); };
					void set(double v) { throw gcnew XbimGeometryFactoryException("2d directions do not support Z values"); }
				};	
				virtual property bool IsNull { bool get() { return ((x * x) + (y * y) ) <= 0; }; };
			};
		}
	}
}


