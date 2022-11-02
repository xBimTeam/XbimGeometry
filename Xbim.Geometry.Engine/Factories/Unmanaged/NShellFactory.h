#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"

#include <TopoDS_Shell.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <vector>
#include "NWireFactory.h"
#include "NFaceFactory.h"

class NShellFactory
{
private:

	NLoggingService* pLoggingService;
	TopoDS_Shell _emptyShell;
	NWireFactory WireFactory;
	NFaceFactory FaceFactory;
public:

	NShellFactory()
	{
		TopoDS_Builder builder;
		builder.MakeShell(_emptyShell); //make an empty solid for failing operations
		pLoggingService = nullptr;

	};
	~NShellFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Shell EmptyShell() { return _emptyShell; }

	TopoDS_Shell BuildConnectedFaceSet(const std::vector<std::vector<std::vector<double>>>& faces, double tolerance, double oneMillimeter);
};

