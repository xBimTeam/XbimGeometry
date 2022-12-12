#ifndef _FlexApp_Application_HeaderFile
#define _FlexApp_Application_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TDocStd_Application.hxx>
#include <TColStd_SequenceOfExtendedString.hxx>
#include <Standard_CString.hxx>
class TDocStd_Document;


class FlexApp_Application;
DEFINE_STANDARD_HANDLE(FlexApp_Application, TDocStd_Application)

//! Implements an Application for the DECAF documents

class FlexApp_Application : public TDocStd_Application
{

public:
	
	//! methods from TDocStd_Application
	//! ================================
	Standard_EXPORT virtual Standard_CString ResourcesName() Standard_OVERRIDE;

	//! Set XCAFDoc_DocumentTool attribute
	Standard_EXPORT virtual void InitDocument(const Handle(TDocStd_Document)& aDoc) const ;

	Standard_EXPORT static Handle(FlexApp_Application) GetApplication();


	//! Dumps the content of me into the stream
	Standard_EXPORT void DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth = -1) const;

	DEFINE_STANDARD_RTTIEXT(FlexApp_Application, TDocStd_Application)
protected:
	Standard_EXPORT FlexApp_Application();
};







#endif // _FlexApp_Application_HeaderFile
