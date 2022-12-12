#pragma once
#include <TopLoc_Location.hxx>
#include <ostream>
class WexBimShape
{

public:
	WexBimShape(int productId, int instanceId, int styleIndex) :
		ProductId(productId), 
		InstanceId(instanceId), 
		StyleIndex(styleIndex)
	{}
	WexBimShape(int productId, int instanceId, int styleIndex, const gp_Trsf & transformation) :
		ProductId(productId),
		InstanceId(instanceId),
		StyleIndex(styleIndex),
		Transformation(transformation)
	{}

	WexBimShape(const WexBimShape& toCopy): 
		ProductId(toCopy.ProductId), 
		StyleIndex(toCopy.StyleIndex), 
		InstanceId(toCopy.InstanceId), 
		Transformation(toCopy.Transformation){}
	int StyleIndex = 0;
	int ProductId=0;
	int InstanceId=0;
	gp_Trsf Transformation;
	void WriteToStream(std::ostream& strm, bool withTransform);
};