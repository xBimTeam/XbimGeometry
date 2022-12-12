#include "WexBimShape.h"
#include <array>
#include <Graphic3d_Vec4.hxx>

void WexBimShape::WriteToStream(std::ostream& strm, bool withTransform)
{
	const unsigned short dummyInstanceId = 0;
	
	strm.write((const char*)&ProductId, sizeof(ProductId));
	strm.write((const char*)&dummyInstanceId, sizeof(dummyInstanceId));
	strm.write((const char*)&InstanceId, sizeof(InstanceId));
	strm.write((const char*)&StyleIndex, sizeof(StyleIndex));

	if (withTransform)
	{
		
		gp_Mat m = Transformation.VectorialPart();
		gp_XYZ t = Transformation.TranslationPart();
		Graphic3d_Vec4d vec0(m.Row(1).X(), m.Row(1).Y(), m.Row(1).Z(), 0.0);
		Graphic3d_Vec4d vec1(m.Row(2).X(), m.Row(2).Y(), m.Row(2).Z(), 0.0);
		Graphic3d_Vec4d vec2(m.Row(3).X(), m.Row(3).Y(), m.Row(3).Z(), 0.0);
		Graphic3d_Vec4d vec3(t.X(), t.Y(), t.Z(), 1.0);

		strm.write((const char*)&vec0, sizeof(vec0));
		strm.write((const char*)&vec1, sizeof(vec1));
		strm.write((const char*)&vec2, sizeof(vec2));
		strm.write((const char*)&vec3, sizeof(vec3));

	}
	
}