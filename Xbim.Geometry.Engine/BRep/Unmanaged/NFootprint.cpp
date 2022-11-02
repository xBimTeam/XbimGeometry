#include "NFootprint.h"

/// <summary>
/// If any point of the loop is inside any of the other loops in the bounds, it is a hole
/// </summary>
/// <param name="loop"></param>
/// <returns></returns>
bool NFootprint::IsHole(const TColgp_Array1OfPnt2d& loop)
{
	//just take the first point
	gp_Pnt2d firstPoint = loop.First();
	for (auto& boundIt = Bounds.cbegin(); boundIt != Bounds.cend(); boundIt++)
	{
		for (auto& polyIt = boundIt->cbegin(); polyIt != boundIt->cend(); polyIt++)
		{
			if (PointInPolygon(firstPoint, *polyIt))
				return true; //it is a hole in one of the boundaries
		}
	}
	return false;
}

void NFootprint::SimplifyBounds()
{
	for (auto& boundIt = Bounds.cbegin(); boundIt != Bounds.cend(); boundIt++)
	{
		for (auto& polyIt = boundIt->begin(); polyIt != boundIt->end(); polyIt++)
		{
			const Handle(Poly_Polygon2D)& polygon = *polyIt;
			SimplifyPolygon(polygon);
		}
	}

}

void NFootprint::SimplifyPolygon(const Handle(Poly_Polygon2D)& poly)
{
	int max_point = poly->NbNodes();
	if (max_point < 3) return; //nothing to remove

	TColgp_Array1OfPnt2d& nodes = poly->ChangeNodes();
	int currentNode = 2; //skip 1, it must remain as must the last point
	TColgp_Array1OfPnt2d nodesCopy(nodes);
	
	for (int i = 2; i < max_point; i++)
	{
		gp_Pnt2d prevPnt = nodes.Value(i - 1);
		gp_Pnt2d nextPnt = nodes.Value(i + 1);
		gp_Pnt2d currentPnt = nodes.Value(i);
		double angle = std::abs(GetAngle(prevPnt, currentPnt, nextPnt));
		if (std::abs(angle - M_PI) < 1e-5) //colinear, skip this current point
			continue;
		nodes.SetValue(currentNode++, currentPnt);
	}
	//set the last Node
	nodes.SetValue(currentNode, nodes.Last());
	nodes.Resize(1, currentNode, true);
}

// Return True if the point is in the polygon.
bool NFootprint::PointInPolygon(const gp_Pnt2d& P, Handle(Poly_Polygon2D) poly)
{
	// Get the angle between the point and the
	// first and last vertices.
	int max_point = poly->NbNodes();
	double total_angle = GetAngle(
		poly->Nodes().Last(),
		P,
		poly->Nodes().First());

	// Add the angles from the point
	// to each other pair of vertices.
	for (int i = 1; i < max_point; i++)
	{
		total_angle += GetAngle(
			poly->Nodes().Value(i),
			P,
			poly->Nodes().Value(i + 1));
	}

	// The total angle should be 2 * PI or -2 * PI if
	// the point is in the polygon and close to zero
	// if the point is outside the polygon. Testing against 1 is good enough
	return (std::abs(total_angle) > 1);
}

// Return the angle ABC.
// Return a value between PI and -PI.
// Note that the value is the opposite of what you might
// expect because Y coordinates increase downward.
double NFootprint::GetAngle(
	const gp_Pnt2d& A,
	const gp_Pnt2d& B,
	const gp_Pnt2d& C)
{
	// Get the dot and cross product.
	double dot_product;
	double cross_product;
	DotProductAndLength(A, B, C, dot_product, cross_product);
	// Calculate the angle.
	return std::atan2(cross_product, dot_product);
}


void NFootprint::DotProductAndLength(
	const gp_Pnt2d& A,
	const gp_Pnt2d& B,
	const gp_Pnt2d& C,
	double& dotProduct,
	double& crossProductLength
)
{
	// Get the vectors' coordinates.
	double BAx = A.X() - B.X();
	double BAy = A.Y() - B.Y();
	double BCx = C.X() - B.X();
	double BCy = C.Y() - B.Y();

	// Calculate the dot product.
	dotProduct = BAx * BCx + BAy * BCy;
	crossProductLength = BAx * BCy - BAy * BCx;
}