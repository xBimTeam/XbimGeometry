#include "NFactoryBase.h"


void NFactoryBase::LogStandardFailure(const Standard_Failure& e, char* additionalMessage)
{
	std::stringstream strm;
	strm << additionalMessage << std::endl;
	e.Print(strm);
	pLoggingService->LogError(strm.str().c_str());
}

void NFactoryBase::LogStandardFailure(const Standard_Failure& e)
{
	std::stringstream strm;
	e.Print(strm);
	pLoggingService->LogError(strm.str().c_str());
}