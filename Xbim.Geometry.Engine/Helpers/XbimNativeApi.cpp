#include "XbimNativeApi.h"

bool XbimNativeApi::FixShell(TopoDS_Shell& shell, double timeOut, std::string& errMsg)
{
	try
	{
		ShapeFix_Shell shellFixer(shell);
		XbimProgressMonitor pi(timeOut);
		if (shellFixer.Perform(pi))
		{
			shell = shellFixer.Shell();
		}
		if (pi.TimedOut())
		{
			errMsg = "ShapeFix_Shell timed out";
			return false;
		}
		return true;
	}
	catch (const Standard_Failure& sf)
	{
		errMsg = sf.GetMessageString();
		if (errMsg.empty())
			errMsg = "Standard Failure in ShapeFix_Shell";
		return false;
	}
}

bool XbimNativeApi::FixShape(TopoDS_Shape& shape, double timeOut, std::string& errMsg)
{
	try
	{
		ShapeFix_Shape shapeFixer(shape);
		XbimProgressMonitor pi(timeOut);
		if (shapeFixer.Perform(pi))
		{
			shape = shapeFixer.Shape();
		}
		if (pi.TimedOut())
		{
			errMsg = "ShapeFix_Shape timed out";
			return false;
		}
		return true;
	}
	catch (const Standard_Failure& sf)
	{
		errMsg = sf.GetMessageString();
		if (errMsg.empty())
			errMsg = "Standard Failure in ShapeFix_Shape";
		return false;
	}
}

bool XbimNativeApi::SewShape(TopoDS_Shape& shape, double tolerance, double timeOut, std::string& errMsg)
{
	try
	{
		BRepBuilderAPI_Sewing seamstress(tolerance);
		seamstress.Add(shape);
		XbimProgressMonitor pi(timeOut);
		seamstress.Perform(pi);
		if (pi.TimedOut())
		{
			errMsg = "Shape sewing timed out";
			return false;
		}
		shape = seamstress.SewedShape();
		return true;
	}
	catch (const Standard_Failure& sf)
	{
		errMsg = sf.GetMessageString();
		if (errMsg.empty())
			errMsg = "Standard Failure in Shape sewing";
		return false;
	}
}

double XbimNativeApi::Length(const Handle(Geom_Curve)& curve)
{

	return  GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(curve));
}


double XbimNativeApi::Length(const Handle(Geom2d_Curve)& curve)
{

	return  GCPnts_AbscissaPoint::Length(Geom2dAdaptor_Curve(curve));
}



bool XbimNativeApi::IsClosed(const TopoDS_Shell& shell)
{
	BRepCheck_Shell checker(shell);
	BRepCheck_Status result = checker.Closed();
	return result == BRepCheck_NoError;
}


gp_Dir XbimNativeApi::NormalDir(const TopoDS_Wire& wire, bool& isValid)
{
	double x = 0, y = 0, z = 0;
	gp_Pnt currentStart, previousEnd, first;
	int count = 0;
	TopLoc_Location loc;
	Standard_Real start = 0, end = 0;

	for (BRepTools_WireExplorer wEx(wire); wEx.More(); wEx.Next())
	{
		const TopoDS_Vertex& v = wEx.CurrentVertex();
		currentStart = BRep_Tool::Pnt(v);
		Handle(Geom_Curve) c3d = BRep_Tool::Curve(wEx.Current(), loc, start, end);
		if (!c3d.IsNull())
		{
			Handle(Geom_Curve) c3dptr = Handle(Geom_Curve)::DownCast(c3d->Transformed(loc.Transformation()));
			Handle(Standard_Type) cType = c3dptr->DynamicType();
			if (cType == STANDARD_TYPE(Geom_Line))
			{
				if (count > 0)
					AddNewellPoint(previousEnd, currentStart, x, y, z);
				else
					first = currentStart;
				previousEnd = currentStart;
			}
			else if (wEx.Current().Closed() && ((cType == STANDARD_TYPE(Geom_Circle)) ||
				(cType == STANDARD_TYPE(Geom_Ellipse)) ||
				(cType == STANDARD_TYPE(Geom_Parabola)) ||
				(cType == STANDARD_TYPE(Geom_Hyperbola)))) //it is a conic
			{
				Handle(Geom_Conic) conic = Handle(Geom_Conic)::DownCast(c3dptr);
				isValid = true;
				return conic->Axis().Direction();

			}
			else if ((cType == STANDARD_TYPE(Geom_Circle)) ||
				(cType == STANDARD_TYPE(Geom_Ellipse)) ||
				(cType == STANDARD_TYPE(Geom_Parabola)) ||
				(cType == STANDARD_TYPE(Geom_Hyperbola)) ||
				(cType == STANDARD_TYPE(Geom_TrimmedCurve)) ||
				(cType == STANDARD_TYPE(Geom_OffsetCurve)) ||
				(cType == STANDARD_TYPE(Geom_BezierCurve)) ||
				(cType == STANDARD_TYPE(Geom_BSplineCurve)))
			{
				// we identify the Us of quadrant points along the curve to compute the normal
				BRepAdaptor_Curve curve(wEx.Current());
				TopAbs_Orientation or = wEx.Current().Orientation();
				double uStart = curve.FirstParameter();
				double uEnd = curve.LastParameter();
				double u0, u1, u2, u3;
				double delta;
				if (or != TopAbs_REVERSED)
				{
					u0 = uStart; // start from the start
					delta = (uEnd - uStart) / 4; // and go forward a bit for each step
				}
				else
				{
					u0 = uEnd; // start from the end
					delta = (uEnd - uStart) / -4; // and go back a bit for each step
				}
				u1 = u0 + delta;
				u2 = u1 + delta;
				u3 = u2 + delta;

				// then we get the points
				gp_Pnt p0; gp_Pnt p1; gp_Pnt p2; gp_Pnt p3;
				curve.D0(u0, p0);
				curve.D0(u1, p1);
				curve.D0(u2, p2);
				curve.D0(u3, p3);

				// then add the points to the newell evaluation
				if (count > 0)
				{
					AddNewellPoint(previousEnd, p0, x, y, z);
					AddNewellPoint(p0, p1, x, y, z);
					AddNewellPoint(p1, p2, x, y, z);
					AddNewellPoint(p2, p3, x, y, z);
					previousEnd = p3;
				}
				else
				{
					first = p0;
					AddNewellPoint(first, p1, x, y, z);
					AddNewellPoint(p1, p2, x, y, z);
					AddNewellPoint(p2, p3, x, y, z);
					previousEnd = p3;
				}
			}
			else //throw AN EXCEPTION
			{
				Standard_Failure::Raise("Unsupported Edge type");
			}
		}
		count++;
	}
	//do the last one
	AddNewellPoint(previousEnd, first, x, y, z);
	try
	{
		gp_Dir dir(x, y, z);
		isValid = true;
		return dir;
	}
	catch (const Standard_Failure&)
	{
		isValid = false;
		return gp::DZ(); //it has no normal, so default to Z
	}

}


void XbimNativeApi::AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double& x, double& y, double& z)
{
	const double& xn = previous.X();
	const double& yn = previous.Y();
	const double& zn = previous.Z();
	const double& xn1 = current.X();
	const double& yn1 = current.Y();
	const double& zn1 = current.Z();
	/*
	Debug::WriteLine("_.LINE");
	Debug::WriteLine("{0},{1},{2}", xn, yn, zn);
	Debug::WriteLine("{0},{1},{2}", xn1, yn1, zn1);
	Debug::WriteLine("");
	*/

	x += (yn - yn1) * (zn + zn1);
	y += (xn + xn1) * (zn - zn1);
	z += (xn - xn1) * (yn + yn1);
	/*
	Debug::WriteLine("-HYPERLINK I O l  {0},{1},{2}", x, y, z);
	Debug::WriteLine("");
	Debug::WriteLine("");
	*/
}

bool XbimNativeApi::RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, std::vector<int> handles, bool closed, double tol)
{
	tol *= tol;
	bool isClosed = false;
	while (true) {
		bool removed = false;
		int n = polygon.Length() - (closed ? 0 : 1);
		for (int i = 1; i <= n; ++i) {
			// wrap around to the first point in case of a closed loop
			int j = (i % polygon.Length()) + 1;
			double dist = polygon.Value(i).SquareDistance(polygon.Value(j));
			if (dist < tol) {
				if (j == 1 && i == n) //the first and last point are the same
					isClosed = true;
				// do not remove the first or last point to
				// maintain connectivity with other wires
				if ((closed && j == 1) || (!closed && j == n))
				{
					polygon.Remove(i);
					handles.erase(handles.begin() + i - 1);
				}
				else
				{
					polygon.Remove(j);
					handles.erase(handles.begin() + j - 1);
				}
				removed = true;
				break;
			}
		}
		if (!removed) break;
	}

	return isClosed;
}



bool XbimNativeApi::RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, bool closed, double tol)
{
	tol *= tol;
	bool isClosed = false;
	while (true) {
		bool removed = false;
		int n = polygon.Length() - (closed ? 0 : 1);
		for (int i = 1; i <= n; ++i) {
			// wrap around to the first point in case of a closed loop
			int j = (i % polygon.Length()) + 1;
			double dist = polygon.Value(i).SquareDistance(polygon.Value(j));
			if (dist < tol) {
				if (j == 1 && i == n) //the first and last point are the same
					isClosed = true;
				// do not remove the first or last point to
				// maintain connectivity with other wires
				if ((closed && j == 1) || (!closed && j == n))
				{
					polygon.Remove(i);
				}
				else
				{
					polygon.Remove(j);
				}
				removed = true;
				break;
			}
		}
		if (!removed) break;
	}

	return isClosed;
}


gp_Vec XbimNativeApi::NewellsNormal(const TColgp_Array1OfPnt& loop, bool& isPlanar)
{
	double x = 0, y = 0, z = 0;
	gp_Pnt previous;
	int count = 0;

	int total = loop.Length();
	for (int i = 0; i <= total; i++)
	{
		gp_Pnt current = i < total ? loop.Value(i + 1) : loop.Value(1);
		if (count > 0)
		{
			double xn = previous.X();
			double yn = previous.Y();
			double zn = previous.Z();
			double xn1 = current.X();
			double yn1 = current.Y();
			double zn1 = current.Z();
			x += (yn - yn1) * (zn + zn1);
			y += (xn + xn1) * (zn - zn1);
			z += (xn - xn1) * (yn + yn1);
		}
		previous = current;
		count++;
	}
	gp_Vec v(x, y, z);
	if (v.Magnitude() >= gp::Resolution())
	{
		isPlanar = true;
		return v.Normalized();
	}
	else
	{
		isPlanar = false;
		return v;
	}


}