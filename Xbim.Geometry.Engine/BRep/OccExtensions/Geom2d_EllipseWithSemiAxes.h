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

	
	Standard_EXPORT Standard_Real ConvertIfcTrimParameter(const Standard_Real ifcTrimValue)
	{
		return _rotated ? ifcTrimValue + M_PI_2 : ifcTrimValue;
	};

	

	//! Creates a new object which is a copy of this line.
	Standard_EXPORT Handle(Geom2d_Geometry) Copy() const Standard_OVERRIDE
	{

		Handle(Geom2d_EllipseWithSemiAxes) L;
		L = new Geom2d_EllipseWithSemiAxes(Position(), MajorRadius(), MinorRadius(), _rotated);
		return L;
	}
};

