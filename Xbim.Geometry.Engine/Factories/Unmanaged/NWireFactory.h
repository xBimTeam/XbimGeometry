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
	NWireFactory()
	{
		pLoggingService = nullptr;
		TopoDS_Builder builder;
		builder.MakeWire(_emptyWire); //make an empty wire for failing operations
	};
	~NWireFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
};

