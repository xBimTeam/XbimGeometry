#pragma once

#include <Geom2d_BoundedCurve.hxx>
#include <Standard_Type.hxx>
#include <Standard_NotImplemented.hxx>
#include <Standard_DefineHandle.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <Standard_Real.hxx>
#include <Precision.hxx>
#include <Standard_DomainError.hxx>
#include <algorithm>
#include <vector>
#include <cmath>

class Geom2d_Polynomial;
DEFINE_STANDARD_HANDLE(Geom2d_Polynomial, Geom2d_BoundedCurve)

class Geom2d_Polynomial : public Geom2d_BoundedCurve {
public:
    Geom2d_Polynomial(
        const gp_Ax22d& placement,
        const std::vector<Standard_Real>& coeffX,
        const std::vector<Standard_Real>& coeffY,
        Standard_Real firstParam = 0.0,
        Standard_Real lastParam = 1.0)
        : _placement(placement),
        _coeffX(coeffX),
        _coeffY(coeffY),
        _firstParam(firstParam),
        _lastParam(lastParam)

    {
        if (_lastParam <= _firstParam) {
            Standard_DomainError::Raise("lastParam must be greater than firstParam.");
        }

        gp_Trsf2d transform;
        transform.SetTransformation(_placement.XAxis());
        transform.Invert();
        _placementTrsf = transform;
    }

    virtual void D0(Standard_Real U, gp_Pnt2d& P) const override {
        U = std::clamp(U, _firstParam, _lastParam);
        Standard_Real x = EvaluatePolynomial(_coeffX, U);
        Standard_Real y = EvaluatePolynomial(_coeffY, U);
        P.SetCoord(x, y);
        P.Transform(_placementTrsf);
    }

    virtual void D1(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const override {
        U = std::clamp(U, _firstParam, _lastParam);
        Standard_Real x = EvaluatePolynomial(_coeffX, U);
        Standard_Real y = EvaluatePolynomial(_coeffY, U);
        Standard_Real dx = EvaluatePolynomialDerivative(_coeffX, U);
        Standard_Real dy = EvaluatePolynomialDerivative(_coeffY, U);
        P.SetCoord(x, y);
        V1.SetCoord(dx, dy);
        P.Transform(_placementTrsf);
        V1.Transform(_placementTrsf);

    }

    virtual void D2(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const override {
        U = std::clamp(U, _firstParam, _lastParam);
        Standard_Real x = EvaluatePolynomial(_coeffX, U);
        Standard_Real y = EvaluatePolynomial(_coeffY, U);
        Standard_Real dx = EvaluatePolynomialDerivative(_coeffX, U);
        Standard_Real dy = EvaluatePolynomialDerivative(_coeffY, U);
        Standard_Real ddx = EvaluatePolynomialSecondDerivative(_coeffX, U);
        Standard_Real ddy = EvaluatePolynomialSecondDerivative(_coeffY, U);
        P.SetCoord(x, y);
        V1.SetCoord(dx, dy);
        V2.SetCoord(ddx, ddy);
        P.Transform(_placementTrsf);
        V1.Transform(_placementTrsf);
        V2.Transform(_placementTrsf);
    }

    virtual void D3(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const override {
        U = std::clamp(U, _firstParam, _lastParam);
        Standard_Real x = EvaluatePolynomial(_coeffX, U);
        Standard_Real y = EvaluatePolynomial(_coeffY, U);
        Standard_Real dx = EvaluatePolynomialDerivative(_coeffX, U);
        Standard_Real dy = EvaluatePolynomialDerivative(_coeffY, U);
        Standard_Real ddx = EvaluatePolynomialSecondDerivative(_coeffX, U);
        Standard_Real ddy = EvaluatePolynomialSecondDerivative(_coeffY, U);
        Standard_Real dddx = EvaluatePolynomialThirdDerivative(_coeffX, U);
        Standard_Real dddy = EvaluatePolynomialThirdDerivative(_coeffY, U);
        P.SetCoord(x, y);
        V1.SetCoord(dx, dy);
        V2.SetCoord(ddx, ddy);
        V3.SetCoord(dddx, dddy);
        P.Transform(_placementTrsf);
        V1.Transform(_placementTrsf);
        V3.Transform(_placementTrsf);
    }

    virtual gp_Vec2d DN(Standard_Real U, Standard_Integer N) const override {
        U = std::clamp(U, _firstParam, _lastParam);
        if (N < 1) {
            Standard_DomainError::Raise("Derivative order must be at least 1.");
        }
        Standard_Real dx = 0.0;
        Standard_Real dy = 0.0;
        if (N == 1) {
            dx = EvaluatePolynomialDerivative(_coeffX, U);
            dy = EvaluatePolynomialDerivative(_coeffY, U);
        }
        else if (N == 2) {
            dx = EvaluatePolynomialSecondDerivative(_coeffX, U);
            dy = EvaluatePolynomialSecondDerivative(_coeffY, U);
        }
        else if (N == 3) {
            dx = EvaluatePolynomialThirdDerivative(_coeffX, U);
            dy = EvaluatePolynomialThirdDerivative(_coeffY, U);
        }
        else {
            dx = EvaluatePolynomialNthDerivative(_coeffX, U, N);
            dy = EvaluatePolynomialNthDerivative(_coeffY, U, N);
        }
        return gp_Vec2d(dx, dy);
    }

    Standard_Real GetCurvatureAt(Standard_Real s) const
    {
        Standard_Real dx = EvaluatePolynomialDerivative(_coeffX, s);
        Standard_Real dy = EvaluatePolynomialDerivative(_coeffY, s);
        Standard_Real ddx = EvaluatePolynomialSecondDerivative(_coeffX, s);
        Standard_Real ddy = EvaluatePolynomialSecondDerivative(_coeffY, s);

        Standard_Real numerator = dx * ddy - dy * ddx;
        Standard_Real denominator = std::pow(dx * dx + dy * dy, 1.5);

        if (denominator == 0.0) {
            Standard_DomainError::Raise("Denominator in curvature computation is zero.");
        }

        Standard_Real curvature = numerator / denominator;
        return curvature;
    }

    gp_Ax22d Placement() const {
        return _placement;
    }

    virtual Standard_Real FirstParameter() const override {
        return _firstParam;
    }

    virtual Standard_Real LastParameter() const override {
        return _lastParam;
    }

    virtual GeomAbs_Shape Continuity() const override {
        return GeomAbs_CN;
    }

    virtual Standard_Boolean IsClosed() const override {
        gp_Pnt2d PStart, PEnd;
        D0(_firstParam, PStart);
        D0(_lastParam, PEnd);
        return PStart.IsEqual(PEnd, Precision::Confusion());
    }

    virtual Standard_Boolean IsPeriodic() const override {
        return Standard_False;
    }

    virtual void Reverse() override {
        Standard_Real temp = _firstParam;
        _firstParam = -_lastParam;
        _lastParam = -temp;
        ReverseCoefficients(_coeffX);
        ReverseCoefficients(_coeffY);
    }

    virtual Standard_Real ReversedParameter(Standard_Real U) const override {
        return _firstParam + _lastParam - U;
    }

    virtual void Transform(const gp_Trsf2d& T) override {
        _placement.Transform(T);
        gp_Trsf2d transform;
        transform.SetTransformation(_placement.XAxis());
        transform.Invert();
        _placementTrsf = transform;
    }

    virtual Handle(Geom2d_Geometry) Copy() const override {
        Handle(Geom2d_Polynomial) copy = new Geom2d_Polynomial(_placement, _coeffX, _coeffY, _firstParam, _lastParam);
        return copy;
    }

    const std::vector<Standard_Real>& CoefficientsX() const {
        return _coeffX;
    }

    const std::vector<Standard_Real>& CoefficientsY() const {
        return _coeffY;
    }

    Standard_Boolean IsCN(Standard_Integer N) const {
        return Standard_True;
    }

    virtual gp_Pnt2d StartPoint() const override {
        gp_Pnt2d P;
        D0(_firstParam, P);
        return P;
    }

    virtual gp_Pnt2d EndPoint() const override {
        gp_Pnt2d P;
        D0(_lastParam, P);
        return P;
    }

private:
    std::vector<Standard_Real> _coeffX;
    std::vector<Standard_Real> _coeffY;
    Standard_Real _firstParam;
    Standard_Real _lastParam;
    gp_Ax22d _placement;
    gp_Trsf2d _placementTrsf;

    Standard_Real EvaluatePolynomial(const std::vector<Standard_Real>& coeffs, Standard_Real t) const {
        Standard_Real value = 0.0;
        for (auto it = coeffs.rbegin(); it != coeffs.rend(); ++it) {
            value = value * t + *it;
        }
        return value;
    }

    Standard_Real EvaluatePolynomialDerivative(const std::vector<Standard_Real>& coeffs, Standard_Real s) const {
        Standard_Real value = 0.0;
        Standard_Integer degree = static_cast<Standard_Integer>(coeffs.size()) - 1;
        for (Standard_Integer i = degree; i >= 1; --i) {
            value = value * s + i * coeffs[i];
        }
        return value;
    }

    Standard_Real EvaluatePolynomialSecondDerivative(const std::vector<Standard_Real>& coeffs, Standard_Real s) const {
        Standard_Real value = 0.0;
        Standard_Integer degree = static_cast<Standard_Integer>(coeffs.size()) - 1;
        for (Standard_Integer i = degree; i >= 2; --i) {
            value = value * s + i * (i - 1) * coeffs[i];
        }
        return value;
    }

    Standard_Real EvaluatePolynomialThirdDerivative(const std::vector<Standard_Real>& coeffs, Standard_Real s) const {
        Standard_Real value = 0.0;
        Standard_Integer degree = static_cast<Standard_Integer>(coeffs.size()) - 1;
        for (Standard_Integer i = degree; i >= 3; --i) {
            value = value * s + i * (i - 1) * (i - 2) * coeffs[i];
        }
        return value;
    }

    Standard_Real EvaluatePolynomialNthDerivative(const std::vector<Standard_Real>& coeffs, Standard_Real s, Standard_Integer N) const {
        if (N < 0) {
            Standard_DomainError::Raise("Derivative order must be non-negative.");
        }
        if (N >= static_cast<Standard_Integer>(coeffs.size())) {
            return 0.0; // Derivative of order greater than degree is zero
        }
        Standard_Real value = 0.0;
        Standard_Integer degree = static_cast<Standard_Integer>(coeffs.size()) - 1;
        for (Standard_Integer i = degree; i >= N; --i) {
            // Compute the coefficient of s^(i - N)
            Standard_Real coeff = coeffs[i];
            for (Standard_Integer k = 0; k < N; ++k) {
                coeff *= (i - k);
            }
            value = value * s + coeff;
        }
        return value;
    }

    void ReverseCoefficients(std::vector<Standard_Real>& coeffs) {
        std::vector<Standard_Real> reversedCoeffs(coeffs.size(), 0.0);
        Standard_Integer degree = static_cast<Standard_Integer>(coeffs.size()) - 1;
        for (Standard_Integer i = 0; i <= degree; ++i) {
            reversedCoeffs[degree - i] = coeffs[i] * ((i % 2 == 0) ? 1.0 : -1.0);
        }
        coeffs = reversedCoeffs;
    }
};
