#pragma once
#include <gp_Pnt2d.hxx>

using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
    namespace Geometry
    {
        namespace BRep
        {
            public ref class X2dPoint : IXPoint
            {
            private:
                double x;
                double y;              
            public:
                X2dPoint() {};
                X2dPoint(double x, double y) : x(x), y(y) {};
                X2dPoint(gp_Pnt2d p) : x(p.X()), y(p.Y()) {};
                virtual property bool Is3d { bool get() { return false; }; };
                virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
                virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
                virtual property double Z { 
                    double get() { throw gcnew XbimGeometryFactoryException("2d points do not support Z values"); };
                    void set(double v) { throw gcnew XbimGeometryFactoryException("2d points do not support Z values"); }
                };
            };
        }
    }
}