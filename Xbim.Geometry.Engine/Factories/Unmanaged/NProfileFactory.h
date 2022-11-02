#pragma once
#include <TopoDS_Compound.hxx>
#include <Geom_Surface.hxx>
#include "../../Services//Unmanaged/NLoggingService.h"
#include <BRep_Builder.hxx>
class NProfileFactory
{
	NLoggingService* pLoggingService;
	BRep_Builder builder;
	TopoDS_Face _emptyFace;
public:

	NProfileFactory()
	{
		pLoggingService = nullptr;
		builder.MakeFace(_emptyFace);
	};
	~NProfileFactory()
	{
		delete pLoggingService;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Compound MakeCompound(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
	TopoDS_Face MakeFace(const TopoDS_Wire& wire, Handle(Geom_Surface) surface);
};

