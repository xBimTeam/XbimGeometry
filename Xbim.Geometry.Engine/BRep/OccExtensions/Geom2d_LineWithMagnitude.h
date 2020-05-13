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
		Geom2d_Line::D0(U, P);
		return P;
	};

	
    Standard_EXPORT Standard_Real ReversedParameter(const Standard_Real U) const Standard_OVERRIDE { return Geom2d_Line::ReversedParameter(U * parameticMagnitude); };



    //! Returns in P the point of parameter U.
    //! P (U) = O + U * Dir where O is the "Location" point of the
    //! line and Dir the direction of the line.
    Standard_EXPORT void D0(const Standard_Real U, gp_Pnt2d& P) const Standard_OVERRIDE { Geom2d_Line::D0(U * parameticMagnitude, P); };


    //! Returns the point P of parameter u and the first derivative V1.
    Standard_EXPORT void D1(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1) const Standard_OVERRIDE { Geom2d_Line::D1(U * parameticMagnitude, P, V1); };


    //! Returns the point P of parameter U, the first and second
    //! derivatives V1 and V2. V2 is a vector with null magnitude
    //! for a line.
    Standard_EXPORT void D2(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2) const Standard_OVERRIDE { Geom2d_Line::D2(U * parameticMagnitude, P, V1, V2); };


    //! V2 and V3 are vectors with null magnitude for a line.
    Standard_EXPORT void D3(const Standard_Real U, gp_Pnt2d& P, gp_Vec2d& V1, gp_Vec2d& V2, gp_Vec2d& V3) const Standard_OVERRIDE { Geom2d_Line::D3(U * parameticMagnitude, P, V1, V2, V3); };


    //! The returned vector gives the value of the derivative for the
    //! order of derivation N.
    //! Raised if N < 1.
    Standard_EXPORT gp_Vec2d DN(const Standard_Real U, const Standard_Integer N) const Standard_OVERRIDE { return Geom2d_Line::DN(U * parameticMagnitude, N); };

   

    Standard_EXPORT virtual Standard_Real ParametricTransformation(const gp_Trsf2d& T) const Standard_OVERRIDE { return Abs(T.ScaleFactor() * parameticMagnitude); };

    //! Creates a new object which is a copy of this line.
    Standard_EXPORT Handle(Geom2d_Geometry) Copy() const Standard_OVERRIDE
    {

        Handle(Geom2d_LineWithMagnitude) L;
        L = new Geom2d_LineWithMagnitude(Position(), parameticMagnitude);
        return L;
    }
};

