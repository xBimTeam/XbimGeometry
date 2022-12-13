#include "FlexShape.h"
#include <TDataStd_BooleanArray.hxx>
#include <TDataStd_NamedData.hxx>

bool FlexShape::IsProduct(const TDF_Label& label)

{
	Handle(TDataStd_BooleanArray)features;
	bool hasFeatures = label.FindAttribute(TDataStd_BooleanArray::GetID(), features);
	if (hasFeatures) return features->Value(3); else return false;
}

bool FlexShape::IsShapeRepresentation(const TDF_Label& label)
{
	Handle(TDataStd_BooleanArray)features;
	bool hasFeatures = label.FindAttribute(TDataStd_BooleanArray::GetID(), features);
	if (hasFeatures) return features->Value(4); else return false;
}

bool FlexShape::IsShapeRepresentationMap(const TDF_Label& label)
{
	Handle(TDataStd_BooleanArray)features;
	bool hasFeatures = label.FindAttribute(TDataStd_BooleanArray::GetID(), features);
	if (hasFeatures) return features->Value(6); else return false;
}

bool FlexShape::IsGeometricRepresentationItem(const TDF_Label& label)
{
	Handle(TDataStd_BooleanArray)features;
	bool hasFeatures = label.FindAttribute(TDataStd_BooleanArray::GetID(), features);
	if (hasFeatures) return features->Value(5); else return false;
}

int FlexShape::IfcId(const TDF_Label& label)
{
	return GetIntAttribute(label, "IfcId");
}

 bool FlexShape::IfcType(const TDF_Label& label,  TCollection_AsciiString& val)
{
	return GetStringAttribute(label, "IfcType", val);
}

 short FlexShape::IfcTypeId(const TDF_Label& label)
 {
	 return GetIntAttribute(label, "IfcTypeId");
 }

int FlexShape::GetIntAttribute(const TDF_Label& label, const TCollection_AsciiString& attributeName)
{
	Handle(TDataStd_NamedData) attributes;
	bool hasAttributes = label.FindAttribute(TDataStd_NamedData::GetID(), attributes);
	if (!hasAttributes) 
		return 0;
	else 
		return attributes->HasInteger(attributeName) ? attributes->GetInteger(attributeName) : 0;

}

bool FlexShape::GetStringAttribute(const TDF_Label& label, const TCollection_AsciiString& attributeName, TCollection_AsciiString& val)
{
	Handle(TDataStd_NamedData) attributes;
	bool hasAttributes = label.FindAttribute(TDataStd_NamedData::GetID(), attributes);
	if (hasAttributes && attributes->HasString(attributeName))
	{
		val = attributes->GetString(attributeName).ToWideString();
		return true;
	}
	else
		return false;

}
