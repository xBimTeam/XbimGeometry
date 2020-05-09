#pragma once
#include <gp_Pnt.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
    namespace Geometry
    {
        namespace BRep
        {
            public ref struct XbimPoint : public IXPoint
            {
            private:
                double x;
                double y;
                double z;
            public:
                XbimPoint() {};
                XbimPoint(double x, double y, double z): x(x), y(y) , z(z) {};
                XbimPoint(const gp_Pnt& p): x(p.X()), y(p.Y()), z(p.Z()) {};
                virtual property bool Is3d { bool get() { return true; };};
                virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
                virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
                virtual property double Z { double get() { return z; }; void set(double v) { z = v; }};
            };
        }
    }
}



