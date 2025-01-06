#pragma once
#include "NFactoryBase.h"

#include <TopoDS_Shell.hxx>

#include <vector>
#include <unordered_map>
#include "NWireFactory.h"
#include "NFaceFactory.h"
#include <TColgp_SequenceOfAx1.hxx>

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
	TopoDS_Shell BuildConnectedFaceSet(const std::vector<std::vector<std::vector<int>>>& faceData, const std::unordered_map<int, gp_XYZ>& points, const std::vector<int>& planeIndices, const TColgp_SequenceOfAx1& planes, double tolerance, double oneMillimeter, bool& needsFixing);
	TopoDS_Shape TrimTopology(const TopoDS_Shape& shape);
	TopoDS_Shape FixShell(TopoDS_Shell& shell, bool& isFixed);

};

