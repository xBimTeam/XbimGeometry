#pragma once
#include <TopoDS_Wire.hxx>
#include <TopoDS_Builder.hxx>
#include "../../Services/Unmanaged/NLoggingService.h"
class NWireFactory
{
private:
	NLoggingService* pLoggingService;
	TopoDS_Wire _emptyWire;
public:
	NWireFactory(NLoggingService* loggingService)
	{
		pLoggingService = loggingService;
		TopoDS_Builder builder;
		builder.MakeWire(_emptyWire); //make an empty wire for failing operations
	};
};

