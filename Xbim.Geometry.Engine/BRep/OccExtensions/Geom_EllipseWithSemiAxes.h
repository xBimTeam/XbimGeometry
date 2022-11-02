#pragma once
#include <Geom_Ellipse.hxx>

class Geom_EllipseWithSemiAxes;
DEFINE_STANDARD_HANDLE(Geom_EllipseWithSemiAxes, Geom_Ellipse)

///Special Elipse that handles the Semi Axis versions in Ifc
class Geom_EllipseWithSemiAxes : public Geom_Ellipse
{
private:
	bool _rotated;
	//private constructor for copying
	Geom_EllipseWithSemiAxes(const gp_Ax2& ax2, double major, double minor, bool rotated) : Geom_Ellipse(ax2, major, minor)
	{
		_rotated = rotated;
	}
public:

	//set the ellipse with compliant major and minor radii
	Geom_EllipseWithSemiAxes(const gp_Ax2& ax2, double semi1, double semi2) : Geom_Ellipse(ax2, Max(semi1, semi2), Min(semi1, semi2))
	{
		if (semi1 <= 0 || semi2 <= 0) throw Standard_ConstructionError(); //just a daft ellipse
		if (semi1 < semi2) // a rotation is requied
		{
			gp_Ax1 zAx = ax2.Axis();
			gp_Ax2 axRot = ax2.Rotated(zAx, -M_PI_2);
			Geom_Ellipse::SetPosition(axRot); //the start is now rotated -90 degrees	
			_rotated = true;
		}
		else
			_rotated = false;
	}
	Standard_EXPORT void Reverse() Standard_OVERRIDE //need to undo the oration, then reverse then redo rotation
	{
		if (_rotated)
		{			
			gp_Ax1 zAx = pos.Axis();
			pos.Rotate(zAx, M_PI_2); //rotate it back			
			gp_Dir vZ = pos.Direction();
			vZ.Reverse();
			pos.SetDirection(vZ);			
			pos.Rotate(zAx, M_PI_2);		
		}
		else
		{
			gp_Dir Vz = pos.Direction();
			Vz.Reverse();
			pos.SetDirection(Vz);
		}
	};
	Standard_EXPORT Standard_Real ReversedParameter(const Standard_Real U) const Standard_OVERRIDE
	{
		return (2. * M_PI - U);
	};

	Standard_EXPORT Standard_Real FirstParameter() const Standard_OVERRIDE
	{
		return _rotated ? M_PI_2 : 0.0;
	};
	Standard_EXPORT Standard_Real LastParameter() const Standard_OVERRIDE
	{
		return _rotated ? 2.0 * M_PI + M_PI_2 : 2.0 * M_PI;;
	};
	//! Returns in P the point of parameter U.
	//! P (U) = O + U * Dir where O is the "Location" point of the
	//! line and Dir the direction of the line.
	Standard_EXPORT void D0(const Standard_Real U, gp_Pnt& P) const Standard_OVERRIDE
	{	
		Geom_Ellipse::D0(_rotated ? U + M_PI_2 : U, P);
	};


	//! Returns the point P of parameter u and the first derivative V1.
	Standard_EXPORT void D1(const Standard_Real U, gp_Pnt& P, gp_Vec& V1) const Standard_OVERRIDE { Geom_Ellipse::D1(_rotated ? U + M_PI_2 : U, P, V1); };


	//! Returns the point P of parameter U, the first and second
	//! derivatives V1 and V2. V2 is a vector with null magnitude
	//! for a line.
	Standard_EXPORT void D2(const Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const Standard_OVERRIDE { Geom_Ellipse::D2(_rotated ? U + M_PI_2 : U, P, V1, V2); };


	//! V2 and V3 are vectors with null magnitude for a line.
	Standard_EXPORT void D3(const Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const Standard_OVERRIDE { Geom_Ellipse::D3(_rotated ? U + M_PI_2 : U, P, V1, V2, V3); };


	//! The returned vector gives the value of the derivative for the
	//! order of derivation N.
	//! Raised if N < 1.
	Standard_EXPORT gp_Vec DN(const Standard_Real U, const Standard_Integer N) const Standard_OVERRIDE { return Geom_Ellipse::DN(_rotated ? U + M_PI_2 : U, N); };

	Standard_EXPORT virtual Standard_Real TransformedParameter(const Standard_Real U, const gp_Trsf& T) const Standard_OVERRIDE
	{
		if (Precision::IsInfinite(U)) return U;
		return U * Abs(T.ScaleFactor()) * 1;
	};



	//Standard_EXPORT virtual Standard_Real ParametricTransformation(const gp_Trsf& T) const Standard_OVERRIDE { return Abs(T.ScaleFactor() * parameticMagnitude); };

	//! Creates a new object which is a copy of this line.
	Standard_EXPORT Handle(Geom_Geometry) Copy() const Standard_OVERRIDE
	{

		Handle(Geom_EllipseWithSemiAxes) L;
		L = new Geom_EllipseWithSemiAxes(Position(), MajorRadius(), MinorRadius(), _rotated);
		return L;
	}
};

