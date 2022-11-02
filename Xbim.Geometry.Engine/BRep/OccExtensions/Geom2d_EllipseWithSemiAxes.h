#pragma once

#include <Geom2d_Ellipse.hxx>

class Geom2d_EllipseWithSemiAxes;

DEFINE_STANDARD_HANDLE(Geom2d_EllipseWithSemiAxes, Geom2d_Ellipse)

class Geom2d_EllipseWithSemiAxes : public Geom2d_Ellipse
{
private:
	bool _rotated;
	Geom2d_EllipseWithSemiAxes(const gp_Ax22d& ax2, double major, double minor, bool rotated) : Geom2d_Ellipse(ax2, major, minor)
	{
		_rotated = rotated;
	}
public:
	//set the ellipse with compliant major and minor radii
	Geom2d_EllipseWithSemiAxes(const gp_Ax22d& ax2, double semi1, double semi2) : Geom2d_Ellipse(ax2, Max(semi1, semi2), Min(semi1, semi2))
	{
		if (semi1 <= 0 || semi2 <= 0) throw Standard_ConstructionError(); //just a daft ellipse
		if (semi1 < semi2) // a rotation is requied
		{
			gp_Ax22d axRot = ax2.Rotated(ax2.Location(), -M_PI_2);
			Geom2d_Ellipse::SetAxis(axRot); //the start is now rotated -90 degrees	
			_rotated = true;
		}
		else
			_rotated = false;
	}
	Standard_EXPORT Standard_Real ReversedParameter(const Standard_Real U) const Standard_OVERRIDE { return ((2. * M_PI - _rotated) ? fmod(U + M_PI_2, M_PI * 2.) : U); };



	//! Returns in P the point of parameter U.
	//! P (U) = O + U * Dir where O is the "Location" point of the
	//! line and Dir the direction of the line.
	Standard_EXPORT void D0(const Standard_Real U, gp_Pnt2d& P) const Standard_OVERRIDE { Geom2d_Ellipse::D0(_rotated ? fmod(U + M_PI_2, M_PI * 2.) : U, P); };


	//! Returns the point P of parameter u and the first derivative V1.
	Standard_EXPORT void D1(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const Standard_OVERRIDE { Geom2d_Ellipse::D1(_rotated ? fmod(U + M_PI_2, M_PI * 2.) : U, P, V1); };


	//! Returns the point P of parameter U, the first and second
	//! derivatives V1 and V2. V2 is a vector with null magnitude
	//! for a line.
	Standard_EXPORT void D2(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const Standard_OVERRIDE { Geom2d_Ellipse::D2(_rotated ? fmod(U + M_PI_2, M_PI * 2.) : U, P, V1, V2); };


	//! V2 and V3 are vectors with null magnitude for a line.
	Standard_EXPORT void D3(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const Standard_OVERRIDE { Geom2d_Ellipse::D3(_rotated ? fmod(U + M_PI_2, M_PI * 2.) : U, P, V1, V2, V3); };


	//! The returned vector gives the value of the derivative for the
	//! order of derivation N.
	//! Raised if N < 1.
	Standard_EXPORT gp_Vec2d DN(const Standard_Real U, const Standard_Integer N) const Standard_OVERRIDE { return Geom2d_Ellipse::DN(_rotated ? fmod(U + M_PI_2, M_PI * 2.) : U, N); };

	/*Standard_EXPORT virtual Standard_Real TransformedParameter(const Standard_Real U, const gp_Trsf& T) const Standard_OVERRIDE
	{
		if (Precision::IsInfinite(U)) return U;
		return U * Abs(T.ScaleFactor()) * parameticMagnitude;
	};*/



	//Standard_EXPORT virtual Standard_Real ParametricTransformation(const gp_Trsf& T) const Standard_OVERRIDE { return Abs(T.ScaleFactor() * parameticMagnitude); };

	//! Creates a new object which is a copy of this line.
	Standard_EXPORT Handle(Geom2d_Geometry) Copy() const Standard_OVERRIDE
	{

		Handle(Geom2d_EllipseWithSemiAxes) L;
		L = new Geom2d_EllipseWithSemiAxes(Position(), MajorRadius(), MinorRadius(), _rotated);
		return L;
	}
};

