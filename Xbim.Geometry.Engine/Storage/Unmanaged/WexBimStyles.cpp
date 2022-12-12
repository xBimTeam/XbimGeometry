#include "WexBimStyles.h"
#include <TDataStd_Name.hxx>
#include <TDF_Label.hxx>
#include <XCAFPrs_Style.hxx>
#include <algorithm>


Graphic3d_Vec4 WexBimStyles::DefineMaterial(const XCAFPrs_Style& theStyle)
{

	if (myStyles.find(theStyle) == myStyles.end()) return myDefaultColour;

	//TODO: we have far more capability here than just argb, we need to consider shininess and eventually PBR and textures, they are all coming through to this point in the style
	XCAFDoc_VisMaterialCommon aCommonMat;
	bool hasMaterial = !theStyle.Material().IsNull()
		&& !theStyle.Material()->IsEmpty();
	if (hasMaterial)
	{
		aCommonMat = theStyle.Material()->ConvertToCommonMaterial();
	}
	else if (!myDefaultStyle.Material().IsNull()
		&& myDefaultStyle.Material()->HasCommonMaterial())
	{
		aCommonMat = myDefaultStyle.Material()->CommonMaterial();
		hasMaterial = true;
	}
	if (hasMaterial)
	{
		return Graphic3d_Vec4((float)aCommonMat.DiffuseColor.Red(), (float)aCommonMat.DiffuseColor.Green(), (float)aCommonMat.DiffuseColor.Blue(), 1.0f - aCommonMat.Transparency);
	}
	else
		return myDefaultColour;


}


// =======================================================================
// function : AddMaterial
// purpose  :
// =======================================================================
int WexBimStyles::AddMaterial(const XCAFPrs_Style& theStyle)
{

	Handle(XCAFDoc_VisMaterial) material = theStyle.Material();

	if (material.IsNull() || material->IsEmpty()) //set to the default
	{
		return 0;
	}
	auto& found = myStyles.find(theStyle);
	if (found != myStyles.cend())
	{
		return found->second;
	}

	Handle(TDataStd_Name) aNodeName;
	if (!material->Label().IsNull())
	{
		myStyles.insert(std::make_pair(theStyle, myNbMaterials));
		myNbMaterials++;
		return myNbMaterials-1;
	}

	return 0;
}


void WexBimStyles::WriteToStream(std::ostream& strm)
{

	for (auto& style : myStyles)
	{
		int styleIndex = style.second;
		strm.write((const char*)&styleIndex, sizeof(styleIndex));
		const XCAFPrs_Style& cafStyle = style.first; //this should never fail
		const Graphic3d_Vec4& colour = DefineMaterial(cafStyle);
		strm.write((const char*)colour.GetData(), sizeof(colour));
	}
}

int WexBimStyles::GetStyleId(const XCAFPrs_Style& theStyle)
{
	int id = myStyles.find(theStyle)->second;
	return id;
}
