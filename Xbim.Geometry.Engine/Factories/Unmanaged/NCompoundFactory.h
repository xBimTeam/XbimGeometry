#pragma once
#include "../../Services/Unmanaged/NLoggingService.h"
#include <BRep_Builder.hxx>


class NCompoundFactory
{
private:
	NLoggingService* pLoggingService;
	TopoDS_Compound _emptyCompound;
public:
	NCompoundFactory()
	{
		pLoggingService = nullptr;
		BRep_Builder builder;
		builder.MakeCompound(_emptyCompound); //make an empty compound for failing operations
		
	};
	~NCompoundFactory()
	{
		if (pLoggingService != nullptr) delete pLoggingService;
		pLoggingService = nullptr;
	};
	void SetLogger(NLoggingService* loggingService) { pLoggingService = loggingService; };
	
};

