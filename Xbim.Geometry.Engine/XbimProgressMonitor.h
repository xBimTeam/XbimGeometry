#pragma once


#ifndef XBIMPROGRESSMONITOR_H
#define XBIMPROGRESSMONITOR_H

# include <Standard_DefineHandle.hxx>
# include <Standard_Macro.hxx>
# include <Message_ProgressIndicator.hxx>
#include <OSD_Timer.hxx>

//DEFINE_STANDARD_HANDLE(XbimProgressIndicator, Message_ProgressIndicator)
class XbimProgressMonitor : public Message_ProgressIndicator
{
private:
	OSD_Timer aTimer;
	Standard_Real maxRunDuration;
	bool timedOut;
public:
	XbimProgressMonitor(Standard_Real maxDurationSeconds, bool startTimer = true);
	virtual Standard_Boolean Show(const Standard_Boolean) { return true; }
	virtual Standard_Boolean UserBreak();
	void StartTimer() { timedOut = false;  aTimer.Start(); }
	void StopTimer() { aTimer.Stop(); }
	Standard_Real ElapsedTime() { return aTimer.ElapsedTime(); }
	bool TimedOut() { return timedOut; }
	/*DEFINE_STANDARD_RTTI(XbimProgressIndicator, Message_ProgressIndicator)*/
};
#endif