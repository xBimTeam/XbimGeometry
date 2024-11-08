#pragma once

#include <Geom_BSplineCurve.hxx>

class Geom_ConvertibleToBSpline;
DEFINE_STANDARD_HANDLE(Geom_ConvertibleToBSpline, Geom_Curve)

class Geom_ConvertibleToBSpline : public Geom_Curve {
public:
    virtual ~Geom_ConvertibleToBSpline() = default;
    virtual  Handle(Geom_BSplineCurve) ToBSpline(int nbPoints) const = 0;

protected:
    Geom_ConvertibleToBSpline() = default;
};
