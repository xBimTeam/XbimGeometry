#pragma once
#include <gp_Vec.hxx>
using namespace Xbim::Geometry::Abstractions;
///represents a vector as a normalised direction and a magnitude
public ref struct XbimVector : public IXVector
{
private:
	double x;
	double y;
	double z;
	double magnitude;

public:
	XbimVector() : x(0.0), y(0.0), z(0.0), magnitude(1.0) {};
	//Create a normalised unit vector in the direction of x, y, z with magnitude 1.0
	//XbimVector(double x, double y, double z)
	//{
	//	gp_Vec v(x, y, z);
	//	magnitude = v.Magnitude();
	//	v.Normalize();
	//	x = v.X();
	//	y = v.Y();
	//	z = v.Z();
	//};
	////Create a normalised unit vector in the direction of x, y, z with magnitude length
	//XbimVector(double x, double y, double z, double len)
	//{
	//	gp_Vec v(x, y, z);
	//	magnitude = len;
	//	v.Normalize();
	//	x = v.X();
	//	y = v.Y();
	//	z = v.Z();
	//};
	// Create a normalised unit vector in the direction of d with magnitude 1.0
	XbimVector(const gp_Dir& d) : XbimVector(gp_Vec(d), 1.0) { };
	// Create a normalised unit vector in the direction of d with magnitude 1.0
	XbimVector(const gp_Vec& v) : XbimVector(v, 1.0) { };
	//Create a normalised unit vector in the direction of v with magnitude length
	XbimVector(const gp_Dir& d, double len) : XbimVector(gp_Vec(d), len) {}
	//Create a normalised unit vector in the direction of v with magnitude length
	XbimVector(const gp_Vec& v, double len)
	{
		magnitude = len;
		gp_Vec nv = v.Normalized();
		x = nv.X();
		y = nv.Y();
		z = nv.Z();
	};
	virtual property bool Is3d { bool get() { return true; }; };
	virtual property double X { double get() { return x; }; void set(double v) { x = v; }};
	virtual property double Y { double get() { return y; }; void set(double v) { y = v; }};
	virtual property double Z { double get() { return z; }; void set(double v) { z = v; }};
	virtual property double Magnitude { double get() { return magnitude; }; void set(double m) { magnitude = m; }};
};

