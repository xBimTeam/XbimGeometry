#include "XMatrix.h"

IXMatrix^ Xbim::Geometry::BRep::XMatrix::Multiply(IXMatrix^ other)
{
	Graphic3d_Mat4d m;
	m.SetValue(0, 0, M11);
	m.SetValue(0, 1, M12);
	m.SetValue(0, 2, M13);
	m.SetValue(0, 3, OffsetX);
	m.SetValue(1, 0, M21);
	m.SetValue(1, 1, M22);
	m.SetValue(1, 2, M23);
	m.SetValue(1, 3, OffsetY);
	m.SetValue(2, 0, M31);
	m.SetValue(2, 1, M32);
	m.SetValue(2, 2, M33);
	m.SetValue(2, 3, OffsetZ);
	m.SetValue(3, 3, M44);
	m.SetValue(3, 0, ScaleX);
	m.SetValue(3, 1, ScaleY);
	m.SetValue(3, 2, ScaleZ);
	Graphic3d_Mat4d res = Ref().Multiplied(m);
	return gcnew XMatrix(res);
}
