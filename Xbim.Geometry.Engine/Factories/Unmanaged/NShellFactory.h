#pragma once
#include "NFactoryBase.h"

#include <TopoDS_Shell.hxx>

#include <vector>
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
	TopoDS_Shell BuildConnectedFaceSet(const std::vector<std::vector<std::vector<double>>>& faces, double tolerance, double oneMillimeter);
};

