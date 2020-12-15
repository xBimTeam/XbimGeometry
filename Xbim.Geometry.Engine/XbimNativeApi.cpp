#include "XbimNativeApi.h"
#include "XbimProgressMonitor.h"
#include <ShapeFix_Shape.hxx>
#include <BRepBuilderAPI_Sewing.hxx>

bool XbimNativeApi::FixShell(TopoDS_Shell& shell, double timeOut, std::string& errMsg)
{
	try
	{
		ShapeFix_Shell shellFixer(shell);
		Handle(XbimProgressMonitor) pi = new XbimProgressMonitor(timeOut);
		if (shellFixer.Perform(pi))
		{
			shell = shellFixer.Shell();
		}
		if (pi->TimedOut())
		{
			errMsg = "ShapeFix_Shell timed out";
			return false;
		}
		return true;
	}
	catch (Standard_Failure sf)
	{
		errMsg = sf.GetMessageString();
		if (errMsg.empty())
			errMsg = "Standard Failure in ShapeFix_Shell";
		return false;
	}
}

bool XbimNativeApi::FixShape(TopoDS_Shape& shape, double timeOut, std::string& errMsg)
{
	try
	{
		ShapeFix_Shape shapeFixer(shape);
		Handle(XbimProgressMonitor) pi = new XbimProgressMonitor(timeOut);
		if (shapeFixer.Perform(pi))
		{
			shape = shapeFixer.Shape();
		}
		if (pi->TimedOut())
		{
			errMsg = "ShapeFix_Shape timed out";
			return false;
		}
		return true;
	}
	catch (Standard_Failure sf)
	{
		errMsg = sf.GetMessageString();
		if (errMsg.empty())
			errMsg = "Standard Failure in ShapeFix_Shape";
		return false;
	}
}

bool XbimNativeApi::SewShape(TopoDS_Shape& shape, double tolerance, double timeOut, std::string& errMsg)
{
	try
	{
		BRepBuilderAPI_Sewing seamstress(tolerance);
		seamstress.Add(shape);
		Handle(XbimProgressMonitor) pi = new XbimProgressMonitor(timeOut);
		seamstress.Perform(pi);
		if (pi->TimedOut())
		{
			errMsg = "Shape sewing timed out";
			return false;
		}
		shape = seamstress.SewedShape();
		return true;
	}
	catch (Standard_Failure sf)
	{
		errMsg = sf.GetMessageString();
		if (errMsg.empty())
			errMsg = "Standard Failure in Shape sewing";
		return false;
	}
}
