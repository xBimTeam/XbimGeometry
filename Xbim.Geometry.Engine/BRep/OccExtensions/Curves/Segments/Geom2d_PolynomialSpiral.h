#pragma once

#include "./Geom2d_Spiral.h"
#include <gp_Ax22d.hxx>
#include <Standard_Real.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <vector>
#include <optional>
#include <Standard_NotImplemented.hxx>
#include <gp_Trsf2d.hxx>
#include <cmath>
#include <algorithm>

class Geom2d_PolynomialSpiral;
DEFINE_STANDARD_HANDLE(Geom2d_PolynomialSpiral, Geom2d_Spiral)

class Geom2d_PolynomialSpiral : public Geom2d_Spiral {
public:
    Geom2d_PolynomialSpiral(
        const gp_Ax22d& placement,
        const std::vector<std::optional<Standard_Real>>& coefficients,
        Standard_Real startParam,
        Standard_Real endParam
    )
        : _placement(placement),
        _coefficients(coefficients),
        _startParam(startParam),
        _endParam(endParam)
    {
        _coefficients.resize(8);
        _integrationSteps = std::max(IntegrationSteps, 1000);
    }

    virtual void D0(Standard_Real U, gp_Pnt2d& P) const override {
        Evaluate(U, P, nullptr, nullptr);
    }

    virtual void D1(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const override {
        Evaluate(U, P, &V1, nullptr);
    }

    virtual void D2(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const override {
        Evaluate(U, P, &V1, &V2);
    }

    virtual void D3(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const override {
        Standard_NotImplemented::Raise("D3 not implemented");
    }


    virtual gp_Vec2d DN(Standard_Real U, Standard_Integer N) const override {
        if (N < 1) {
            Standard_DomainError::Raise("Derivative order must be at least 1.");
        }

        if (N == 1) {
            gp_Pnt2d P;
            gp_Vec2d V1;
            D1(U, P, V1);
            return V1;
        }
        else if (N == 2) {
            gp_Pnt2d P;
            gp_Vec2d V1, V2;
            D2(U, P, V1, V2);
            return V2;
        }
        else {
            Standard_NotImplemented::Raise("DN not implemented for N > 2");
            return gp_Vec2d();
        }
    }

    Standard_Real GetCurvatureAt(Standard_Real s) const override {
        s = std::max(_startParam, std::min(s, _endParam));
        return CalculateKappa(s);
    }

    virtual Standard_Real FirstParameter() const override {
        return _startParam;
    }

    virtual Standard_Real LastParameter() const override {
        return _endParam;
    }

    virtual gp_Ax22d Placement() const override {
        return _placement;
    }

    virtual Standard_Boolean IsClosed() const override {
        return Standard_False;
    }

    virtual Standard_Boolean IsPeriodic() const override {
        return Standard_False;
    }

    virtual Handle(Geom2d_Geometry) Copy() const override {
        return new Geom2d_PolynomialSpiral(_placement, _coefficients, _startParam, _endParam);
    }

    virtual void Reverse() override {
        Standard_NotImplemented::Raise("Geom2d_PolynomialSpiral::Reverse not implemented");
    }

    virtual Standard_Real ReversedParameter(Standard_Real U) const override {
        return -U;
    }

    virtual void Transform(const gp_Trsf2d& T) override {
        _placement.Transform(T);
    }

    virtual GeomAbs_Shape Continuity() const override {
        return GeomAbs_CN;
    }

    virtual Standard_Boolean IsCN(Standard_Integer N) const override {
        return Standard_True;
    }

    virtual gp_Pnt2d StartPoint() const override {
        gp_Pnt2d P;
        D0(_startParam, P);
        return P;
    }

    virtual gp_Pnt2d EndPoint() const override {
        gp_Pnt2d P;
        D0(_endParam, P);
        return P;
    }

    Standard_Integer GetIntervalsCount() const override {
        return _integrationSteps;
    }

private:
    gp_Ax22d _placement;
    std::vector<std::optional<Standard_Real>> _coefficients; // A0 to A7
    Standard_Real _startParam;
    Standard_Real _endParam;
    Standard_Integer _integrationSteps;

    void Evaluate(Standard_Real U, gp_Pnt2d& P, gp_Vec2d* V1 = nullptr, gp_Vec2d* V2 = nullptr) const {
        
        Standard_Real t = std::max(_startParam, std::min(U, _endParam));
        Standard_Real dt = (t - _startParam) / _integrationSteps;

        Standard_Real x = 0.0;
        Standard_Real y = 0.0;

        for (int i = 0; i <= _integrationSteps; ++i) {
            Standard_Real ti = _startParam + i * dt;
            Standard_Real theta_i = CalculateTheta(ti);
            Standard_Real weight = (i == 0 || i == _integrationSteps) ? 1.0 : (i % 2 == 0 ? 2.0 : 4.0);
            x += weight * cos(theta_i);
            y += weight * sin(theta_i);
        }

        x *= dt / 3.0;
        y *= dt / 3.0;

        gp_Pnt2d pntLocal(x, y);
        gp_Trsf2d transform;
        transform.SetTransformation(_placement.XAxis());
        P = pntLocal.Transformed(transform);

        if (V1 || V2) {
            Standard_Real theta = CalculateTheta(t);
            Standard_Real dxdt = cos(theta);
            Standard_Real dydt = sin(theta);
            gp_Vec2d tangentLocal(dxdt, dydt);
            gp_Vec2d tangent = tangentLocal.Transformed(transform);
            if (V1) {
                *V1 = tangent;
            }

            if (V2) {
                Standard_Real kappa = CalculateKappa(t);

                Standard_Real ddxdt = -kappa * sin(theta);
                Standard_Real ddydt = kappa * cos(theta);
                gp_Vec2d normalLocal(ddxdt, ddydt);
                gp_Vec2d normal = normalLocal.Transformed(transform);
                *V2 = normal;
            }
        }
    }

    Standard_Real CalculateTheta(Standard_Real t) const {
        Standard_Real theta = 0.0;

        if (_coefficients[0].has_value()) {
            Standard_Real A0 = _coefficients[0].value();
            theta += t / A0;
        }

        if (_coefficients[1].has_value()) {
            Standard_Real A1 = _coefficients[1].value();
            theta += (A1 * t * t) / (2.0 * std::fabs(std::pow(A1, 3)));
        }

        if (_coefficients[2].has_value()) {
            Standard_Real A2 = _coefficients[2].value();
            theta += (std::pow(t, 3)) / (3.0 * std::pow(A2, 3));
        }

        if (_coefficients[3].has_value()) {
            Standard_Real A3 = _coefficients[3].value();
            theta += (A3 * std::pow(t, 4)) / (4.0 * std::fabs(std::pow(A3, 5)));
        }

        if (_coefficients[4].has_value()) {
            Standard_Real A4 = _coefficients[4].value();
            theta += (std::pow(t, 5)) / (5.0 * std::pow(A4, 5));
        }

        if (_coefficients[5].has_value()) {
            Standard_Real A5 = _coefficients[5].value();
            theta += (A5 * std::pow(t, 6)) / (6.0 * std::fabs(std::pow(A5, 7)));
        }

        if (_coefficients[6].has_value()) {
            Standard_Real A6 = _coefficients[6].value();
            theta += (std::pow(t, 7)) / (7.0 * std::pow(A6, 7));
        }

        if (_coefficients[7].has_value()) {
            Standard_Real A7 = _coefficients[7].value();
            theta += (A7 * std::pow(t, 8)) / (8.0 * std::fabs(std::pow(A7, 9)));
        }

        return theta;
    }

    Standard_Real CalculateKappa(Standard_Real t) const {
        Standard_Real kappa = 0.0;

        if (_coefficients[0].has_value()) {
            Standard_Real A0 = _coefficients[0].value();
            kappa += 1.0 / A0;
        }

        if (_coefficients[1].has_value()) {
            Standard_Real A1 = _coefficients[1].value();
            kappa += (A1 * t) / std::fabs(std::pow(A1, 3));
        }

        if (_coefficients[2].has_value()) {
            Standard_Real A2 = _coefficients[2].value();
            kappa += (t * t) / std::pow(A2, 3);
        }

        if (_coefficients[3].has_value()) {
            Standard_Real A3 = _coefficients[3].value();
            kappa += (A3 * std::pow(t, 3)) / std::fabs(std::pow(A3, 5));
        }

        if (_coefficients[4].has_value()) {
            Standard_Real A4 = _coefficients[4].value();
            kappa += (std::pow(t, 4)) / std::pow(A4, 5);
        }

        if (_coefficients[5].has_value()) {
            Standard_Real A5 = _coefficients[5].value();
            kappa += (A5 * std::pow(t, 5)) / std::fabs(std::pow(A5, 7));
        }

        if (_coefficients[6].has_value()) {
            Standard_Real A6 = _coefficients[6].value();
            kappa += (std::pow(t, 6)) / std::pow(A6, 7);
        }

        if (_coefficients[7].has_value()) {
            Standard_Real A7 = _coefficients[7].value();
            kappa += (A7 * std::pow(t, 7)) / std::fabs(std::pow(A7, 9));
        }

        return kappa;
    }
};
