#pragma once

#include "./Geom2d_Spiral.h"
#include <Standard_Type.hxx>
#include <Standard_NotImplemented.hxx>
#include <Standard_DefineHandle.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <Standard_Real.hxx>
#include <gp_Ax22d.hxx>
#include <gp_Trsf2d.hxx>
#include <Standard_DomainError.hxx>
#include <algorithm>
#include <cmath>

class Geom2d_SineSpiral;
DEFINE_STANDARD_HANDLE(Geom2d_SineSpiral, Geom2d_Spiral)

class Geom2d_SineSpiral : public Geom2d_Spiral {
public:
	Geom2d_SineSpiral(
		const gp_Ax22d& placement,
		Standard_Real SineTerm,
		Standard_Real LinearTerm = 0.0,
		Standard_Real ConstantTerm = 0.0,
		Standard_Real firstParam = 0.0,
		Standard_Real lastParam = 1.0)
		: _placement(placement),
		_sineTerm(SineTerm),
		_linearTerm(LinearTerm),
		_constantTerm(ConstantTerm),
		_startParam(firstParam),
		_endParam(lastParam)
	{
		_integrationSteps = std::max(IntegrationSteps, 1000);
		if (_endParam <= _startParam) {
			Standard_DomainError::Raise("lastParam must be greater than firstParam.");
		}
		_length = _endParam - _startParam;
		gp_Trsf2d transform;
		transform.SetTransformation(_placement.XAxis());
		transform.Invert();
		_placementTrsf = transform;
	}


	virtual void D0(Standard_Real U, gp_Pnt2d& P) const override {
		U = std::clamp(U, _startParam, _endParam);
		Standard_Real x, y;
		Evaluate(U, x, y);
		P.SetCoord(x, y);
	}

	virtual void D1(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const override {
		D0(U, P);
		Standard_Real theta = GetHeadingAt(U);
		V1.SetCoord(std::cos(theta), std::sin(theta));
		V1.Transform(_placementTrsf);
	}

	virtual void D2(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const override {
		D1(U, P, V1);
		Standard_Real K = GetCurvatureAt(U);
		V2.SetCoord(-K * V1.Y(), K * V1.X());
		V2.Transform(_placementTrsf);
	}

	virtual void D3(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const override {
		Standard_NotImplemented::Raise("D3 not implemented");
	}

	virtual gp_Vec2d DN(Standard_Real U, Standard_Integer N) const override {
		if (N < 1) {
			Standard_DomainError::Raise("Derivative order must be at least 1.");
		}

		gp_Pnt2d P;
		gp_Vec2d V1, V2;
		if (N == 1) {
			D1(U, P, V1);
			return V1;
		}
		else if (N == 2) {
			D2(U, P, V1, V2);
			return V2;
		}
		else {
			Standard_NotImplemented::Raise("Geom2d_SineSpiral::DN not implemented for N > 2");
			return gp_Vec2d();
		}
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

	virtual GeomAbs_Shape Continuity() const override {
		return GeomAbs_CN;
	}

	virtual Standard_Boolean IsClosed() const override {
		return Standard_False;
	}

	virtual Standard_Boolean IsPeriodic() const override {
		return Standard_False;
	}

	virtual void Reverse() override {
		Standard_NotImplemented::Raise("Geom2d_SineSpiral::Reverse not implemented");
	}

	virtual Standard_Real ReversedParameter(Standard_Real U) const override {
		return _startParam + _endParam - U;
	}

	virtual void Transform(const gp_Trsf2d& T) override {
		_placement.Transform(T);
		gp_Trsf2d transform;
		transform.SetTransformation(_placement.XAxis());
		transform.Invert();
		_placementTrsf = transform;
	}


	virtual Standard_Boolean IsCN(Standard_Integer N) const override {
		return Standard_True;
	}

	virtual Handle(Geom2d_Geometry) Copy() const override {
		return new Geom2d_SineSpiral(_placement, _sineTerm, _linearTerm, _constantTerm, _startParam, _endParam);
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

	Standard_Real GetCurvatureAt(Standard_Real s) const {
		auto sign = [](double v) -> double { return v ? v / fabs(v) : 1.0; };
		auto firstTerm = _constantTerm != 0 ? _length / _constantTerm : 0.0;
		auto secondTerm = _linearTerm != 0 ? sign(_linearTerm) * pow(_length / (_linearTerm), 2.0) * (s / _length) : 0.0;
		auto thirdTerm = (_length / (_sineTerm)) * sin(2 * M_PI * s / _length);
		return firstTerm + secondTerm + thirdTerm;
	}

	Standard_Integer GetIntervalsCount() const override {
		return _integrationSteps;
    }

private:
	gp_Ax22d _placement;
	gp_Trsf2d _placementTrsf;
	Standard_Real _sineTerm;
	Standard_Real _linearTerm;
	Standard_Real _constantTerm;
	Standard_Real _startParam;
	Standard_Real _endParam;
	Standard_Real _length;
	Standard_Integer _integrationSteps;

	Standard_Real GetHeadingAt(Standard_Real s) const {
		auto sign = [](double v) -> double { return v ? v / fabs(v) : 1.0; };
		auto firstTerm = _constantTerm != 0 ? s / _constantTerm : 0.0;
		auto secondTerm = _linearTerm != 0 ? 0.5 * sign(_linearTerm) * pow(s / (_linearTerm), 2.0) : 0.0;
		auto thirdTerm = -1 * (_length / (2 * M_PI * _sineTerm)) * (cos(2 * M_PI * s / _length) - 1);
		return firstTerm + secondTerm + thirdTerm;
	}


	void Evaluate(Standard_Real s, Standard_Real& x, Standard_Real& y) const {
		s = std::clamp(s, _startParam, _endParam);
		Standard_Integer N = _integrationSteps;
		Standard_Real ds = (s - _startParam) / N;

		x = 0.0;
		y = 0.0;

		Standard_Real sumX = 0.0;
		Standard_Real sumY = 0.0;

		for (Standard_Integer i = 0; i <= N; ++i) {
			Standard_Real si = _startParam + i * ds;
			Standard_Real theta = GetHeadingAt(si);
			double weight = (i == 0 || i == _integrationSteps) ? 1 : (i % 2 == 0 ? 2 : 4);
			sumX += weight * std::cos(theta);
			sumY += weight * std::sin(theta);
		}

		x = (ds / 3.0) * sumX;
		y = (ds / 3.0) * sumY;

		gp_Pnt2d P(x, y);
		P.Transform(_placementTrsf);
		x = P.X();
		y = P.Y();
	}

};