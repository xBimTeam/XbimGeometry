#pragma once

#include <Geom2d_BoundedCurve.hxx>

class Geom2d_Spiral;
DEFINE_STANDARD_HANDLE(Geom2d_Spiral, Geom2d_BoundedCurve)

class Geom2d_Spiral : public Geom2d_BoundedCurve {
public:
    static inline Standard_Integer IntegrationSteps;

    virtual ~Geom2d_Spiral() = default;
    virtual Standard_Real GetCurvatureAt(Standard_Real s) const = 0;
    virtual gp_Ax22d Placement() const = 0;
    virtual Standard_Integer GetIntervalsCount() const = 0;

protected:
    Geom2d_Spiral() = default;
};
