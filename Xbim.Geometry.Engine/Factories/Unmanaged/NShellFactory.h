#pragma once
#include "NFactoryBase.h"

#include <TopoDS_Shell.hxx>

#include <vector>
#include <unordered_map>
#include "NWireFactory.h"
#include "NFaceFactory.h"

class NShellFactory : NFactoryBase
{
private:

	NWireFactory WireFactory;
	NFaceFactory FaceFactory;
public:
	virtual void SetLogger(WriteLog lFunc) override
	{
		NFactoryBase::SetLogger(lFunc);
		WireFactory.SetLogger(lFunc);
		FaceFactory.SetLogger(lFunc);
	};
	TopoDS_Shell BuildConnectedFaceSet(const std::vector<std::vector<std::vector<int>>>& faceData, const std::unordered_map<int, gp_XYZ>& points, double tolerance, double oneMillimeter, bool& needsFixing);
};

