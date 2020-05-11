#pragma once
#include <Geom_Line.hxx>

class Geom_LineWithMagnitude;
DEFINE_STANDARD_HANDLE(Geom_LineWithMagnitude, Geom_Line)

class Geom_LineWithMagnitude :
	public Geom_Line
{
private:
	double parameticMagnitude;
public:
	Geom_LineWithMagnitude(gp_Pnt pnt, gp_Dir dir, double magnitude) : Geom_Line(pnt, dir)
	{
		parameticMagnitude = magnitude;
	}
	Geom_LineWithMagnitude(gp_Ax1 pos, double magnitude) : Geom_Line(pos)
	{
		parameticMagnitude = magnitude;
	}
	double Magnitude() { return parameticMagnitude; };
	//Retrieves the point at parameter U
	gp_Pnt GetPoint(const Standard_Real U) const
	{
		gp_Pnt P;
		Geom_Line::D0(U * parameticMagnitude, P);
		return P;
	};

	void FirstDerivative(const Standard_Real U, gp_Pnt& P, gp_Vec& V1) const
	{
		return Geom_Line::D1(U * parameticMagnitude, P, V1);
	}
	Handle(Geom_Geometry) Copy() const {

		Handle(Geom_LineWithMagnitude) L;
		L = new Geom_LineWithMagnitude(Position(), parameticMagnitude);
		return L;
	}
};

