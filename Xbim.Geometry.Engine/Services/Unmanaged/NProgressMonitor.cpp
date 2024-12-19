#include "NProgressMonitor.h"

NProgressMonitor::NProgressMonitor(Standard_Real maxDurationSeconds, bool startTimer) :
	Message_ProgressRange()
{
	maxRunDuration = maxDurationSeconds;
	if (startTimer) StartTimer();
}

Standard_Boolean NProgressMonitor::UserBreak()
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