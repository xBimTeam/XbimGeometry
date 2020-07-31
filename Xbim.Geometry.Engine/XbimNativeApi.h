#pragma once
#include <ShapeFix_Shell.hxx>

class XbimNativeApi
{
public:
	static bool FixShell(TopoDS_Shell& shell, double timeOut, std::string& errMsg);
	static bool FixShape(TopoDS_Shape& shape, double timeOut, std::string& errMsg);
	static bool SewShape(TopoDS_Shape& shape, double tolerance, double timeOut, std::string& errMsg);
};

