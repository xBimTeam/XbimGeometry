#pragma once
#include <TDF_Label.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_AsciiString.hxx>
class NBRepDocument
{
public:
	static TDF_Label FindVisualMaterial(const Handle(TDocStd_Document)& theDoc,
		const TCollection_AsciiString& theKey);
};

