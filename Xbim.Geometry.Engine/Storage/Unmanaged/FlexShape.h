#pragma once
#include <TDF_Label.hxx>
class FlexShape
{
public:
	static bool IsProduct(const TDF_Label& label);
	static bool IsShapeRepresentation(const TDF_Label& label);
	static bool IsShapeRepresentationMap(const TDF_Label& label);
	static bool IsGeometricRepresentationItem(const TDF_Label& label);
	static int IfcId(const TDF_Label& label);
	static bool IfcType(const TDF_Label& label, TCollection_AsciiString& val);
	static short IfcTypeId(const TDF_Label& label);
	static int GetIntAttribute(const TDF_Label& label, const TCollection_AsciiString& attributeName);
	static bool GetStringAttribute(const TDF_Label& label, const TCollection_AsciiString& attributeName, TCollection_AsciiString& val);
};

