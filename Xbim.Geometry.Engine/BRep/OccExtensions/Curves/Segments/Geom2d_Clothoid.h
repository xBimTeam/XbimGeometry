#pragma once
#include <Geom2d_BoundedCurve.hxx>
#include <gp_Ax2.hxx>
#include <Standard_Type.hxx>
#include <Standard_DefineHandle.hxx>
#include <Standard_Real.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax22d.hxx>
#include <math.h>
#include <Standard_NotImplemented.hxx>
#include "./Geom2d_Spiral.h"

class Geom2d_Clothoid;
DEFINE_STANDARD_HANDLE(Geom2d_Clothoid, Geom2d_Spiral)

class Geom2d_Clothoid : public Geom2d_Spiral {
public:
	Geom2d_Clothoid(const gp_Ax22d& placement, Standard_Real clothoidConstant,
		Standard_Real startParam, Standard_Real endParam)
		: _placement(placement),
		_clothoidConstant(clothoidConstant),
		_startParam(startParam),
		_endParam(endParam)
	{
		_integrationSteps = std::max(IntegrationSteps, 1000);
	}

	virtual void D0(Standard_Real U, gp_Pnt2d& P) const {
		Evaluate(U, P, nullptr);
	}
	virtual void D1(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const override {
		Evaluate(U, P, &V1);
	}

	virtual void D2(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const override {
		Evaluate(U, P, &V1, &V2);
	}

	virtual void D3(Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const override {
		Standard_NotImplemented::Raise("Geom2d_Clothoid::D3 not implemented");
	}

	virtual gp_Vec2d DN(Standard_Real U, Standard_Integer N) const {

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
			Standard_NotImplemented::Raise("Geom2d_Clothoid::DN not implemented for N > 2");
			return gp_Vec2d();
		}
	}

	Standard_Real GetCurvatureAt(Standard_Real s) const override {
		// the curve curvature at some ac length can be analytically obtained by this eqn
		return _clothoidConstant * s / fabs(pow(_clothoidConstant, 3));
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
		return new Geom2d_Clothoid(_placement, _clothoidConstant, _startParam, _endParam);
	}

	virtual void Reverse() override {
		Standard_NotImplemented::Raise("Geom2d_Clothoid::Reverse not implemented");
	}

	virtual Standard_Real ReversedParameter(Standard_Real U) const override {
		return -U;
	}

	virtual void Transform(const gp_Trsf2d& T) override {
		_placement.Transform(T);
	}

	virtual Standard_Real TransformedParameter(Standard_Real U, const gp_Trsf2d& T) const override {
		// Since the clothoid is parameterized by arc length, and arc length is invariant under transformation, return U
		return U;
	}

	virtual Standard_Real ParametricTransformation(const gp_Trsf2d& T) const override {
		// The parametric transformation ratio
		return 1.0;
	}

	virtual GeomAbs_Shape Continuity() const override {
		return GeomAbs_CN;
	}

	virtual Standard_Boolean IsCN(Standard_Integer N) const override {
		return Standard_True;
	}

	virtual gp_Pnt2d StartPoint() const override
	{
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
	Standard_Real _clothoidConstant;
	Standard_Real _startParam;
	Standard_Real _endParam;
	Standard_Integer _integrationSteps;


	void Evaluate(Standard_Real U, gp_Pnt2d& P, gp_Vec2d* V1 = nullptr, gp_Vec2d* V2 = nullptr) const
	{

		U = std::max(_startParam, std::min(U, _endParam));

		Standard_Real t = U / (_clothoidConstant * sqrt(M_PI));

		Standard_Real fresnelC, fresnelS;
		FresnelIntegrals(t, fresnelC, fresnelS);

		Standard_Real x = _clothoidConstant * sqrt(M_PI) * fresnelC;
		Standard_Real y = _clothoidConstant * sqrt(M_PI) *  fresnelS;
		gp_Pnt2d pntLocal(x, y);


		gp_Trsf2d transform;
		transform.SetTransformation(_placement.XAxis());
		transform.Invert();
		P = pntLocal.Transformed(transform);

		if (V1 || V2) 
		{
			Standard_Real theta = 0.5 * (_clothoidConstant / abs(_clothoidConstant)) * M_PI * t * t;

			// First derivative
			Standard_Real dxds = cos(theta);
			Standard_Real dyds = sin(theta);
			gp_Vec2d tangentLocal(dxds, dyds);

			gp_Vec2d tangent = tangentLocal.Transformed(transform);
			tangent.Normalize();
			if (V1) {
				*V1 = tangent;
			}

			// Second derivative
			if (V2) {
				Standard_Real dtheta_ds = t / _clothoidConstant;
				Standard_Real ddxds = -dtheta_ds * sin(theta);
				Standard_Real ddyds = dtheta_ds * cos(theta);
				gp_Vec2d normalLocal(ddxds, ddyds);
				gp_Vec2d normal = normalLocal.Transformed(transform);
				normal.Normalize();
				*V2 = normal;
			}
		}
	}


	void FresnelIntegrals(Standard_Real t, Standard_Real& C, Standard_Real& S) const {

		// Solving the Fresnel integrals using Simpson's rule numerical approximation
		double dt = t / _integrationSteps; // Interval width
		double sumC = 0.0;
		double sumS = 0.0;

		for (int i = 0; i <= _integrationSteps; ++i) {
			double t = i * dt;
			double angle = 0.5 * (_clothoidConstant / abs(_clothoidConstant))* M_PI * t * t;
			// The weight of of the ith interval (Simpson's rule has alternating weight between 2 and 4)
			// Only the first and last interval don't have weights
			double weight = (i == 0 || i == _integrationSteps) ? 1 : (i % 2 == 0 ? 2 : 4);
			sumC += weight * cos(angle);
			sumS += weight * sin(angle);
		}

		double integralC = (dt / 3.0) * sumC;
		double integralS = (dt / 3.0) * sumS;

		C = integralC;
		S = integralS;
	}


};