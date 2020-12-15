#include "XbimProgressMonitor.h"


//IMPLEMENT_STANDARD_HANDLE(XbimProgressIndicator, Message_ProgressIndicator)
//IMPLEMENT_STANDARD_RTTIEXT(XbimProgressIndicator, Message_ProgressIndicator)


XbimProgressMonitor::XbimProgressMonitor(Standard_Real maxDurationSeconds, bool startTimer) :
	Message_ProgressIndicator()
{
	maxRunDuration = maxDurationSeconds;
	if (startTimer) StartTimer();
}

Standard_Boolean XbimProgressMonitor::UserBreak()
{
	
	if (ElapsedTime() > maxRunDuration)
	{
		StopTimer();
		timedOut = true;
		return true;
	}
	else
		return false;
}


