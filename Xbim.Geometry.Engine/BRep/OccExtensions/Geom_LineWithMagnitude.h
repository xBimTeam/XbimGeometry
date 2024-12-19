#pragma once
#include <Geom_Line.hxx>
#include <Precision.hxx>
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
	//gp_Pnt GetPoint(const Standard_Real U) const
	//{
	//	gp_Pnt P;
	//	D0(U, P);
	//	return P;
	//};

	//
 //   Standard_EXPORT Standard_Real ReversedParameter(const Standard_Real U) const Standard_OVERRIDE { return Geom_Line::ReversedParameter(U * parameticMagnitude); };



 //   //! Returns in P the point of parameter U.
 //   //! P (U) = O + U * Dir where O is the "Location" point of the
 //   //! line and Dir the direction of the line.
 //   Standard_EXPORT void D0(const Standard_Real U, gp_Pnt& P) const Standard_OVERRIDE { Geom_Line::D0(U * parameticMagnitude, P); };


 //   //! Returns the point P of parameter u and the first derivative V1.
 //   Standard_EXPORT void D1(const Standard_Real U, gp_Pnt& P, gp_Vec& V1) const Standard_OVERRIDE { Geom_Line::D1(U * parameticMagnitude, P, V1); };


 //   //! Returns the point P of parameter U, the first and second
 //   //! derivatives V1 and V2. V2 is a vector with null magnitude
 //   //! for a line.
 //   Standard_EXPORT void D2(const Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const Standard_OVERRIDE { Geom_Line::D2(U * parameticMagnitude, P, V1, V2); };


 //   //! V2 and V3 are vectors with null magnitude for a line.
 //   Standard_EXPORT void D3(const Standard_Real U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const Standard_OVERRIDE { Geom_Line::D3(U * parameticMagnitude, P, V1, V2, V3); };


 //   //! The returned vector gives the value of the derivative for the
 //   //! order of derivation N.
 //   //! Raised if N < 1.
 //   Standard_EXPORT gp_Vec DN(const Standard_Real U, const Standard_Integer N) const Standard_OVERRIDE { return Geom_Line::DN(U * parameticMagnitude,N); };

 //   Standard_EXPORT virtual Standard_Real TransformedParameter(const Standard_Real U, const gp_Trsf& T) const Standard_OVERRIDE 
 //   {
 //       if (Precision::IsInfinite(U)) return U;
 //       return U * Abs(T.ScaleFactor()) * parameticMagnitude;
 //   };

 //  
 //   Standard_EXPORT virtual Standard_Real ParametricTransformation(const gp_Trsf& T) const Standard_OVERRIDE { return Abs(T.ScaleFactor() * parameticMagnitude); };

	Standard_EXPORT Standard_Real ConvertIfcTrimParameter(const Standard_Real ifcTrimValue)
	{
		return ifcTrimValue * parameticMagnitude;
	};
    //! Creates a new object which is a copy of this line.
    Standard_EXPORT Handle(Geom_Geometry) Copy() const Standard_OVERRIDE
    {

        Handle(Geom_LineWithMagnitude) L;
        L = new Geom_LineWithMagnitude(Position(), parameticMagnitude);
        return L;
    }

};

