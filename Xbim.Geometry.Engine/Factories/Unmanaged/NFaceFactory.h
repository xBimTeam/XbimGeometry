#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"
#include <BRep_Builder.hxx>
#include <gp_Pln.hxx>

class NFaceFactory
{
private:
	NLoggingService* pLoggingService;
	TopoDS_Face _emptyFace;
public:
	NFaceFactory()
	{
		pLoggingService = nullptr;
		BRep_Builder builder;
		builder.MakeFace(_emptyFace); //make an empty face for failing operations
	};
	~NFaceFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Face BuildProfileDef(gp_Pln plane, const TopoDS_Wire& wire);
	gp_Vec Normal(const TopoDS_Face& face);
};

