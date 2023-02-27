#include "FlexApp_Application.h"

#include <Resource_Manager.hxx>
#include <Standard_Dump.hxx>
#include <TDF_Label.hxx>
#include <TDocStd_Document.hxx>
#include <TPrsStd_DriverTable.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFPrs_Driver.hxx>

IMPLEMENT_STANDARD_RTTIEXT(FlexApp_Application, TDocStd_Application)
static Handle(FlexApp_Application) locApp;
Handle(FlexApp_Application) FlexApp_Application::GetApplication()
{
	
	if (locApp.IsNull()) locApp = new FlexApp_Application;
	return locApp;
}

//=======================================================================
//function : FlexApp_Application
//purpose  : 
//=======================================================================ko

FlexApp_Application::FlexApp_Application()
{
	// register driver for presentation
	Handle(TPrsStd_DriverTable) table = new TPrsStd_DriverTable;;
	table->AddDriver(XCAFPrs_Driver::GetID(), new XCAFPrs_Driver);
	
	
}

//=======================================================================
//function : ResourcesName
//purpose  : 
//=======================================================================

Standard_CString FlexApp_Application::ResourcesName()
{
	return Standard_CString("XCAF");	
}

//=======================================================================
//function : InitDocument
//purpose  : 
//=======================================================================

void FlexApp_Application::InitDocument(const Handle(TDocStd_Document)& aDoc) const
{
	Handle(XCAFDoc_DocumentTool) docTool =  XCAFDoc_DocumentTool::Set(aDoc->Main());
}



//=======================================================================
//function : DumpJson
//purpose  : 
//=======================================================================
void FlexApp_Application::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
	OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

		OCCT_DUMP_BASE_CLASS(theOStream, theDepth, TDocStd_Application)
}

