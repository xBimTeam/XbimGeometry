#include "ColourRGB.h"




bool Xbim::Geometry::Visual::ColourRGB::Equals(Object^ obj)
{
    IXColourRGB^ rgb = dynamic_cast<ColourRGB^>(obj);
    if (rgb == nullptr) return false;
    return Equals(this, rgb);
}

int Xbim::Geometry::Visual::ColourRGB::GetHashCode()
{
    return GetHashCode((IXColourRGB^)this);
}

bool Xbim::Geometry::Visual::ColourRGB::Equals(ColourRGB^ x, ColourRGB^ y)
{
    return Equals((IXColourRGB^)x, (IXColourRGB^)x);
}

int Xbim::Geometry::Visual::ColourRGB::GetHashCode(ColourRGB^ obj)
{
    return GetHashCode((IXColourRGB^)obj);
}

bool Xbim::Geometry::Visual::ColourRGB::Equals(IXColourRGB^ x, IXColourRGB^ y)
{
    if (x == nullptr && y == nullptr) return true;
    if (x == nullptr || y == nullptr) return false;
    bool equal = (x->Red, x->Green, x->Blue) == (y->Red, y->Green, y->Blue);
    return equal;
}

int Xbim::Geometry::Visual::ColourRGB::GetHashCode(IXColourRGB^ obj)
{
    int r = (int)(255 * obj->Red);
    int g = (int)(255 * obj->Green);
    int b = (int)(255 * obj->Blue);
    int hash = b + (g << 8) + (r << 16);
    return hash;
}