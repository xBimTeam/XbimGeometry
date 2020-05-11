#pragma once
#include <Geom2d_Line.hxx>

class Geom2d_LineWithMagnitude;
DEFINE_STANDARD_HANDLE(Geom2d_LineWithMagnitude, Geom2d_Line)

class Geom2d_LineWithMagnitude :
	public Geom2d_Line
{
private:
	double parameticMagnitude;
public:
	Geom2d_LineWithMagnitude(gp_Pnt2d pnt, gp_Dir2d dir, double magnitude) : Geom2d_Line(pnt, dir)
	{
		parameticMagnitude = magnitude;
	}

	Geom2d_LineWithMagnitude(gp_Ax2d pos, double magnitude) : Geom2d_Line(pos)
	{
		parameticMagnitude = magnitude;
	}

	double Magnitude() { return parameticMagnitude; };
	//Retrieves the point at parameter U
	gp_Pnt2d GetPoint(const Standard_Real U) const
	{
		gp_Pnt2d P;
		Geom2d_Line::D0(U * parameticMagnitude, P);
		return P;
	};

	void FirstDerivative(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const
	{
		return Geom2d_Line::D1(U * parameticMagnitude, P, V1);
	}
	Handle(Geom2d_Geometry) Copy() const {

		Handle(Geom2d_LineWithMagnitude) L;
		L = new Geom2d_LineWithMagnitude(Position(), parameticMagnitude);
		return L;
	}
};

