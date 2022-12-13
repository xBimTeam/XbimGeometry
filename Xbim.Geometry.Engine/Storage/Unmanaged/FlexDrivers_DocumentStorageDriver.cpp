#include "FlexDrivers_DocumentStorageDriver.h"

#include <BinMDF_ADriverTable.hxx>
#include "FlexDrivers.h"
#include <Message_Messenger.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(FlexDrivers_DocumentStorageDriver, BinDrivers_DocumentStorageDriver)

//=======================================================================
//function : 
//purpose  :
//=======================================================================
FlexDrivers_DocumentStorageDriver::FlexDrivers_DocumentStorageDriver() {
}

//=======================================================================
//function : 
//purpose  :
//=======================================================================
Handle(BinMDF_ADriverTable) FlexDrivers_DocumentStorageDriver::AttributeDrivers(const Handle(Message_Messenger)& theMsgDriver) {
	return  FlexDrivers::AttributeDrivers(theMsgDriver);
}

