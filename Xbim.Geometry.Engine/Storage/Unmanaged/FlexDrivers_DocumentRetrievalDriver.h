

#ifndef _FlexDrivers_DocumentRetrievalDriver_HeaderFile
#define _FlexDrivers_DocumentRetrievalDriver_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <BinDrivers_DocumentRetrievalDriver.hxx>
class BinMDF_ADriverTable;
class Message_Messenger;


class FlexDrivers_DocumentRetrievalDriver;
DEFINE_STANDARD_HANDLE(FlexDrivers_DocumentRetrievalDriver, BinDrivers_DocumentRetrievalDriver)


class FlexDrivers_DocumentRetrievalDriver : public BinDrivers_DocumentRetrievalDriver
{

public:


	//! Constructor
	Standard_EXPORT FlexDrivers_DocumentRetrievalDriver();

	Standard_EXPORT virtual Handle(BinMDF_ADriverTable) AttributeDrivers(const Handle(Message_Messenger)& theMsgDriver) Standard_OVERRIDE;

	DEFINE_STANDARD_RTTIEXT(FlexDrivers_DocumentRetrievalDriver, BinDrivers_DocumentRetrievalDriver)

protected:




private:




};







#endif // _FlexDrivers_DocumentRetrievalDriver_HeaderFile

