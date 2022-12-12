
#ifndef _FlexDrivers_DocumentStorageDriver_HeaderFile
#define _FlexDrivers_DocumentStorageDriver_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <BinDrivers_DocumentStorageDriver.hxx>
class BinMDF_ADriverTable;
class Message_Messenger;


class FlexDrivers_DocumentStorageDriver;
DEFINE_STANDARD_HANDLE(FlexDrivers_DocumentStorageDriver, BinDrivers_DocumentStorageDriver)


class FlexDrivers_DocumentStorageDriver : public BinDrivers_DocumentStorageDriver
{

public:


	//! Constructor
	Standard_EXPORT FlexDrivers_DocumentStorageDriver();

	Standard_EXPORT virtual Handle(BinMDF_ADriverTable) AttributeDrivers(const Handle(Message_Messenger)& theMsgDriver) Standard_OVERRIDE;




	DEFINE_STANDARD_RTTIEXT(FlexDrivers_DocumentStorageDriver, BinDrivers_DocumentStorageDriver)

protected:




private:




};







#endif // _FlexDrivers_DocumentStorageDriver_HeaderFile

