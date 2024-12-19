#pragma once
#include <OSD_Timer.hxx>
#include <Message_ProgressScope.hxx>
class NProgressMonitor : public Message_ProgressRange
{
private:
	OSD_Timer aTimer;
	Standard_Real maxRunDuration;
	bool timedOut = false;
public:
	NProgressMonitor(Standard_Real maxDurationSeconds, bool startTimer = true);
	virtual Standard_Boolean Show(const Standard_Boolean) { return true; }
	virtual Standard_Boolean UserBreak();
	void StartTimer() { timedOut = false;  aTimer.Start(); }
	void StopTimer() { aTimer.Stop(); }
	Standard_Real ElapsedTime() { return aTimer.ElapsedTime(); }
	bool TimedOut() { return timedOut; }
	
};