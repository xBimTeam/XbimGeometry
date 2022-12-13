
#ifndef _FlexDrivers_HeaderFile
#define _FlexDrivers_HeaderFile

#include <Standard_Handle.hxx>

class Standard_Transient;
class Standard_GUID;
class BinMDF_ADriverTable;
class Message_Messenger;
class BinXCAFDrivers_DocumentStorageDriver;
class BinXCAFDrivers_DocumentRetrievalDriver;
class TDocStd_Application;

class FlexDrivers
{
public:

	Standard_EXPORT static const Handle(Standard_Transient)& Factory(const Standard_GUID& theGUID);

	//! Defines format "Flex BinXCAF" and registers its read and write drivers
	//! in the specified application
	Standard_EXPORT static void DefineFormat(const Handle(TDocStd_Application)& theApp);

	//! Creates the table of drivers of types supported
	Standard_EXPORT static Handle(BinMDF_ADriverTable) AttributeDrivers(const Handle(Message_Messenger)& MsgDrv);
};

#endif // _FlexDrivers_HeaderFile


