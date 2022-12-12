#include "FlexDrivers.h"


#include <BinDrivers.hxx>
#include <BinMDF_ADriverTable.hxx>
#include <BinMXCAFDoc.hxx>

#include "FlexDrivers_DocumentRetrievalDriver.h"
#include "FlexDrivers_DocumentStorageDriver.h"

#include <Message_Messenger.hxx>
#include <Plugin_Macro.hxx>
#include <Standard_Failure.hxx>
#include <Standard_GUID.hxx>
#include <TDocStd_Application.hxx>

static Standard_GUID FlexStorageDriver(  "66314A2A-87A1-4CD0-8A40-A8C88C372984");
static Standard_GUID FlexRetrievalDriver("66314A2A-87A1-4CD0-8A40-A8C88C372984");

//=======================================================================
//function :
//purpose  : 
//=======================================================================
const Handle(Standard_Transient)& FlexDrivers::Factory(const Standard_GUID& theGUID) {

    if (theGUID == FlexStorageDriver)
    {
#ifdef OCCT_DEBUG
        std::cout << "BinXCAFDrivers : Storage Plugin" << std::endl;
#endif
        static Handle(Standard_Transient) model_sd =
            new FlexDrivers_DocumentStorageDriver;
        return model_sd;
    }

    if (theGUID == FlexRetrievalDriver)
    {
#ifdef OCCT_DEBUG
        std::cout << "BinXCAFDrivers : Retrieval Plugin" << std::endl;
#endif
        static Handle(Standard_Transient) model_rd =
            new FlexDrivers_DocumentRetrievalDriver;
        return model_rd;
    }


    throw Standard_Failure("FlexDrivers : unknown GUID");
}

//=======================================================================
//function : DefineFormat
//purpose  : 
//=======================================================================
void FlexDrivers::DefineFormat(const Handle(TDocStd_Application)& theApp)
{
    theApp->DefineFormat("BinXCAF", "Binary XCAF Document", "xbf",
        new FlexDrivers_DocumentRetrievalDriver,
        new FlexDrivers_DocumentStorageDriver);
}



//=======================================================================
//function :
//purpose  : 
//=======================================================================
Handle(BinMDF_ADriverTable) FlexDrivers::AttributeDrivers(const Handle(Message_Messenger)& aMsgDrv) {
    // Standard Drivers
    Handle(BinMDF_ADriverTable) aTable = BinDrivers::AttributeDrivers(aMsgDrv);

    // XCAF Drivers
    BinMXCAFDoc::AddDrivers(aTable, aMsgDrv);

    return aTable;
}

PLUGIN(FlexDrivers)
