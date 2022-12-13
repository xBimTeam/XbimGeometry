#include "FlexDrivers_DocumentRetrievalDriver.h"


#include <BinMDF_ADriverTable.hxx>
#include "FlexDrivers.h"
#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(FlexDrivers_DocumentRetrievalDriver, BinDrivers_DocumentRetrievalDriver)

//=======================================================================
//function : 
//purpose  :
//=======================================================================
FlexDrivers_DocumentRetrievalDriver::FlexDrivers_DocumentRetrievalDriver() {
}

//=======================================================================
//function : 
//purpose  :
//=======================================================================
Handle(BinMDF_ADriverTable) FlexDrivers_DocumentRetrievalDriver::AttributeDrivers(const Handle(Message_Messenger)& theMsgDriver) {
	return FlexDrivers::AttributeDrivers(theMsgDriver);
}