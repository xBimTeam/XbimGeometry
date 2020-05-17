#pragma once
#include <TopoDS_Solid.hxx>
#include "NLoggingService.h"
class NShapeService
{
private:
	NLoggingService* pLoggingService;

public:
	NShapeService()
	{
	}
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	TopoDS_Shape UnifyDomain(const TopoDS_Shape& toFix);
	TopoDS_Solid UnifyDomain(const TopoDS_Solid& toFix);
};

