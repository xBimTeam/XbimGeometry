#pragma once
#include <XCAFPrs_Style.hxx>
#include <unordered_map>


struct StyleHasher
{
	std::size_t operator()(const XCAFPrs_Style& k) const
	{
		return k.HashCode(k,10000); //warning there is a hard limit of 10000 styles, seems unlikley to be exceeded
	}
};
class WexBimStyles
{
public:
	WexBimStyles(const XCAFPrs_Style& theStyle) : myNbMaterials(1), myDefaultStyle(theStyle)
	{
		myStyles.insert(std::make_pair(myDefaultStyle, 0));
		XCAFDoc_VisMaterialCommon& aCommonMat = theStyle.Material()->ConvertToCommonMaterial();
		myDefaultColour = Graphic3d_Vec4((float)aCommonMat.DiffuseColor.Red(), (float)aCommonMat.DiffuseColor.Green(), (float)aCommonMat.DiffuseColor.Blue(), 1.0f - aCommonMat.Transparency);
	}

	int AddMaterial(const XCAFPrs_Style& theStyle);

	
	int NumberOfMaterials() { return myNbMaterials; }

	void WriteToStream(std::ostream& strm);
	int GetStyleId(const XCAFPrs_Style& theStyle);

private:

	
	std::unordered_map<XCAFPrs_Style, int, StyleHasher> myStyles;
	Standard_Integer        myNbMaterials;       //!< number of registered materials	
	XCAFPrs_Style           myDefaultStyle;      //!< default material definition to be used for nodes with only color defined	
	Graphic3d_Vec4			myDefaultColour;
	//!  method actually defining the material in the  scene file).
	Standard_EXPORT   Graphic3d_Vec4 DefineMaterial(const XCAFPrs_Style& theStyle);

};