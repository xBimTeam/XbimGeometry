#pragma once
#include <Poly_Polygon2D.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <vector>


class NFootprint
{
public:
	NFootprint() : IsClose(true), MinZ(0), MaxZ(0)
	{
	}
	///A list of list of rings/loops (allows for inner loops)
	std::vector<std::vector<Handle(Poly_Polygon2D)>> Bounds;
	double MinZ;
	double MaxZ;
	bool IsClose;
	/// <summary>
	/// If any point of the loop is inside any of the other loops in the bounds, it is a hole
	/// </summary>
	/// <param name="loop"></param>
	/// <returns></returns>
	bool IsHole(const TColgp_Array1OfPnt2d& loop);
	/// <summary>
	/// Iterates all bounds and removed coliner segements and any duplicate points
	/// </summary>
	void SimplifyBounds();
	void SimplifyPolygon( const Handle(Poly_Polygon2D)& polygon);
private:
	// Return True if the point is in the polygon.
	static bool PointInPolygon(const gp_Pnt2d& P, Handle(Poly_Polygon2D) poly);

	// Return the angle ABC.
// Return a value between PI and -PI.
// Note that the value is the opposite of what you might
// expect because Y coordinates increase downward.
	static double GetAngle(
		const gp_Pnt2d& A,
		const gp_Pnt2d& B,
		const gp_Pnt2d& C);


	static void DotProductAndLength(
		const gp_Pnt2d& A,
		const gp_Pnt2d& B,
		const gp_Pnt2d& C,
		double& dotProduct,
		double& crossProductLength
	);

};
