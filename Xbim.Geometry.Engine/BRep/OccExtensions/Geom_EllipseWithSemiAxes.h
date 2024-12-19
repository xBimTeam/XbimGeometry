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
	

	Standard_EXPORT Standard_Real ConvertIfcTrimParameter(const Standard_Real ifcTrimValue)
	{
		return _rotated ? ifcTrimValue + M_PI_2 : ifcTrimValue;
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

