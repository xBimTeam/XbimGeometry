#include "BRepDocumentManager.h"

#include "BRepDocument.h"
//#include <TDocStd_Document.hxx>
//#include <chrono>
//#include <STEPCAFControl_Reader.hxx>

#include <BinTools.hxx>
#include "../BRep/XShape.h"
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Geometry::Exceptions;


class membuf : public std::basic_streambuf<char> {
public:
	membuf(const uint8_t* p, size_t l) {
		setg((char*)p, (char*)p, (char*)p + l);
	}
};
class memstream : public std::istream {
public:
	memstream(const unsigned char * p, size_t l) :
		std::istream(&_buffer),
		_buffer(p, l) {
		rdbuf(&_buffer);
	}

private:
	membuf _buffer;
};

namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{
			int BRepDocumentManager::InSession(System::String^ filePath)
			{
				System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filePath);
				try
				{
					const char* pAnsiPath = static_cast<const char*>(p.ToPointer());
					return  Ref()->IsInSession(pAnsiPath);
					
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Document Manager Error, " + msg);
				}
				finally
				{
					System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
				}
			}

			IXBRepDocument^ BRepDocumentManager::GetDocument(int docIndex)
			{
				Handle(TDocStd_Document) aDoc;
				Ref()->GetDocument(docIndex, aDoc);
				return gcnew BRepDocument(aDoc);
			}
			IXBRepDocument^ BRepDocumentManager::Open(array<System::Byte>^ bytes)
			{
				try
				{
					Handle(TDocStd_Document) aDoc;
					pin_ptr<System::Byte> p = &bytes[1];   // entire array is now pinned
					unsigned  char* cp = p;
					std::string data((const char*)cp);
					std::istringstream input(data);
					PCDM_ReaderStatus status = Ref()->Open(input, aDoc);
					switch (status)
					{
					case PCDM_RS_OK:
						return gcnew BRepDocument(aDoc);
					case PCDM_RS_NoDriver:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoDriver");
						break;
					case PCDM_RS_UnknownFileDriver:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UnknownFileDriver");
						break;
					case PCDM_RS_OpenError:
						_loggingService->LogError("Open Document Failed: PCDM_RS_OpenError");
						break;
					case PCDM_RS_NoVersion:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoVersion");
						break;
					case PCDM_RS_NoSchema:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoSchema");
						break;
					case PCDM_RS_NoDocument:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoDocument");
						break;
					case PCDM_RS_ExtensionFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_ExtensionFailure");
						break;
					case PCDM_RS_WrongStreamMode:
						_loggingService->LogError("Open Document Failed: PCDM_RS_WrongStreamMode");
						break;
					case PCDM_RS_FormatFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_FormatFailure");
						break;
					case PCDM_RS_TypeFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_TypeFailure");
						break;
					case PCDM_RS_TypeNotFoundInSchema:
						_loggingService->LogError("Open Document Failed: PCDM_RS_TypeNotFoundInSchema");
						break;
					case PCDM_RS_UnrecognizedFileFormat:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UnrecognizedFileFormat");
						break;
					case PCDM_RS_MakeFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_MakeFailure");
						break;
					case PCDM_RS_PermissionDenied:
						_loggingService->LogError("Open Document Failed: PCDM_RS_PermissionDenied");
						break;
					case PCDM_RS_DriverFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_DriverFailure");
						break;
					case PCDM_RS_AlreadyRetrievedAndModified:
						_loggingService->LogError("Open Document Failed: PCDM_RS_AlreadyRetrievedAndModified");
						break;
					case PCDM_RS_AlreadyRetrieved:
						_loggingService->LogError("Open Document Failed: PCDM_RS_AlreadyRetrieved");
						break;
					case PCDM_RS_UnknownDocument:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UnknownDocument");
						break;
					case PCDM_RS_WrongResource:
						_loggingService->LogError("Open Document Failed: PCDM_RS_WrongResource");
						break;
					case PCDM_RS_ReaderException:
						_loggingService->LogError("Open Document Failed: PCDM_RS_ReaderException");
						break;
					case PCDM_RS_NoModel:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoModel");
						break;
					case PCDM_RS_UserBreak:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UserBreak");
						break;
					default:
						_loggingService->LogError("Open Document Failed: Unknown Error");
						break;
					}
					Standard_Failure::Raise("Failed to Open BRep store, see logs for details");
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Document Manager Error, " + msg);
				}

				return nullptr;
			}

			IXBRepDocument^ BRepDocumentManager::Open(System::String^ filePath)
			{

				System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filePath);
				bool lockWasTaken = false;
				try
				{
					Monitor::Enter(_lockObject, lockWasTaken);

					const char* pAnsiPath = static_cast<const char*>(p.ToPointer());

					int id = 0;// Ref()->IsInSession(pAnsiPath);
					if (id != 0)
						throw gcnew FileLoadException("Document cannot be opened multiple times in a session: " + filePath);
					Handle(TDocStd_Document) aDoc;
					PCDM_ReaderStatus status = Ref()->Open(pAnsiPath, aDoc);

					id = 0;// Ref()->IsInSession(pAnsiPath);
					switch (status)
					{
					case PCDM_RS_OK:
						return gcnew BRepDocument(aDoc);
					case PCDM_RS_NoDriver:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoDriver");
						break;
					case PCDM_RS_UnknownFileDriver:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UnknownFileDriver");
						break;
					case PCDM_RS_OpenError:
						_loggingService->LogError("Open Document Failed: PCDM_RS_OpenError");
						break;
					case PCDM_RS_NoVersion:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoVersion");
						break;
					case PCDM_RS_NoSchema:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoSchema");
						break;
					case PCDM_RS_NoDocument:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoDocument");
						break;
					case PCDM_RS_ExtensionFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_ExtensionFailure");
						break;
					case PCDM_RS_WrongStreamMode:
						_loggingService->LogError("Open Document Failed: PCDM_RS_WrongStreamMode");
						break;
					case PCDM_RS_FormatFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_FormatFailure");
						break;
					case PCDM_RS_TypeFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_TypeFailure");
						break;
					case PCDM_RS_TypeNotFoundInSchema:
						_loggingService->LogError("Open Document Failed: PCDM_RS_TypeNotFoundInSchema");
						break;
					case PCDM_RS_UnrecognizedFileFormat:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UnrecognizedFileFormat");
						break;
					case PCDM_RS_MakeFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_MakeFailure");
						break;
					case PCDM_RS_PermissionDenied:
						_loggingService->LogError("Open Document Failed: PCDM_RS_PermissionDenied");
						break;
					case PCDM_RS_DriverFailure:
						_loggingService->LogError("Open Document Failed: PCDM_RS_DriverFailure");
						break;
					case PCDM_RS_AlreadyRetrievedAndModified:
						_loggingService->LogError("Open Document Failed: PCDM_RS_AlreadyRetrievedAndModified");
						break;
					case PCDM_RS_AlreadyRetrieved:
						_loggingService->LogError("Open Document Failed: PCDM_RS_AlreadyRetrieved");
						break;
					case PCDM_RS_UnknownDocument:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UnknownDocument");
						break;
					case PCDM_RS_WrongResource:
						_loggingService->LogError("Open Document Failed: PCDM_RS_WrongResource");
						break;
					case PCDM_RS_ReaderException:
						_loggingService->LogError("Open Document Failed: PCDM_RS_ReaderException");
						break;
					case PCDM_RS_NoModel:
						_loggingService->LogError("Open Document Failed: PCDM_RS_NoModel");
						break;
					case PCDM_RS_UserBreak:
						_loggingService->LogError("Open Document Failed: PCDM_RS_UserBreak");
						break;
					default:
						_loggingService->LogError("Open Document Failed: Unknown Error");
						break;
					}
					Standard_Failure::Raise("Failed to Open BRep store, see logs for details");
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Document Manager Error, " + msg);
				}
				finally
				{
					System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
					if (lockWasTaken) Monitor::Exit(_lockObject);
				}
				return nullptr;
			}

			IXBRepDocument^ BRepDocumentManager::NewDocument()
			{

				try
				{
					Handle(TDocStd_Document) aDoc;
					Ref()->NewDocument("BinXCAF", aDoc);
					return gcnew BRepDocument(aDoc);
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Document Manager Error, " + msg);
				}

				return nullptr;
			}

			IXBRepDocument^ BRepDocumentManager::NewDocument(double oneMeter, double precision)
			{
				IXBRepDocument^ doc = NewDocument();
				if (doc != nullptr)
				{
					doc->ConversionFactorForOneMeter = oneMeter;
					doc->PrecisionFactor = precision;
				}
				return doc;
			}

			bool BRepDocumentManager::Save(IXBRepDocument^ document)
			{

				bool lockWasTaken = false;
				try
				{
					Monitor::Enter(_lockObject, lockWasTaken);
					BRepDocument^ doc = dynamic_cast<BRepDocument^>(document);
					if (doc == nullptr) //this is a programmatic error
						throw gcnew System::Exception("Document  cannot be converted to Xbim BRepDocument");
					PCDM_StoreStatus status = Ref()->Save(doc->Ref());
					switch (status)
					{
					case PCDM_SS_OK:
						return true;
					case PCDM_SS_DriverFailure:
						_loggingService->LogError("Save Document Failed: PCDM_SS_DriverFailure");
						break;
					case PCDM_SS_WriteFailure:
						_loggingService->LogError("Save Document Failed: PCDM_SS_WriteFailure");
						break;
					case PCDM_SS_Failure:
						_loggingService->LogError("Save Document Failed: PCDM_SS_Failure");
						break;
					case PCDM_SS_Doc_IsNull:
						_loggingService->LogError("Save Document Failed: PCDM_SS_Doc_IsNull");
						break;
					case PCDM_SS_No_Obj:
						_loggingService->LogError("Save Document Failed: PCDM_SS_No_Obj");
						break;
					case PCDM_SS_Info_Section_Error:
						_loggingService->LogError("Save Document Failed: PCDM_SS_Info_Section_Error");
						break;
					case PCDM_SS_UserBreak:
						_loggingService->LogError("Save Document Failed: PCDM_SS_UserBreak");
						break;
					default:
						_loggingService->LogError("Save Document Failed: Unknown Error");
						break;
					}
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					_loggingService->LogError("Save Document Failed: " + msg);
				}
				finally
				{
					if (lockWasTaken) Monitor::Exit(_lockObject);
				}
				return false;
			}

			bool BRepDocumentManager::SaveAs(System::String^ filePath, IXBRepDocument^ document)
			{
				System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filePath);
				bool lockWasTaken = false;
				try
				{
					Monitor::Enter(_lockObject, lockWasTaken);
					BRepDocument^ doc = dynamic_cast<BRepDocument^>(document);
					if (doc == nullptr) //this is a programmatic error
						throw gcnew System::Exception("Document cannot be converted to Xbim BRepDocument");
					const char* pAnsiPath = static_cast<const char*>(p.ToPointer());
					PCDM_StoreStatus status = Ref()->SaveAs(doc->Ref(), pAnsiPath);
					switch (status)
					{
					case PCDM_SS_OK:
						return true;
					case PCDM_SS_DriverFailure:
						_loggingService->LogError("Save Document Failed: PCDM_SS_DriverFailure");
						break;
					case PCDM_SS_WriteFailure:
						_loggingService->LogError("Save Document Failed: PCDM_SS_WriteFailure");
						break;
					case PCDM_SS_Failure:
						_loggingService->LogError("Save Document Failed: PCDM_SS_Failure");
						break;
					case PCDM_SS_Doc_IsNull:
						_loggingService->LogError("Save Document Failed: PCDM_SS_Doc_IsNull");
						break;
					case PCDM_SS_No_Obj:
						_loggingService->LogError("Save Document Failed: PCDM_SS_No_Obj");
						break;
					case PCDM_SS_Info_Section_Error:
						_loggingService->LogError("Save Document Failed: PCDM_SS_Info_Section_Error");
						break;
					case PCDM_SS_UserBreak:
						_loggingService->LogError("Save Document Failed: PCDM_SS_UserBreak");
						break;
					default:
						_loggingService->LogError("Save Document Failed: Unknown Error");
						break;
					}
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					_loggingService->LogError("Save Document Failed: " + msg);
				}
				finally
				{
					System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
					if (lockWasTaken) Monitor::Exit(_lockObject);
				}
				return false;
			}

			array<System::Byte>^ BRepDocumentManager::ToArray(IXBRepDocument^ document)
			{
				std::stringstream output;

				try
				{
					BRepDocument^ doc = dynamic_cast<BRepDocument^>(document);
					if (doc == nullptr) //this is a programmatic error
						throw gcnew System::Exception("Document  cannot be converted to Xbim BRepDocument");
					PCDM_StoreStatus status = Ref()->SaveAs(doc->Ref(), output);
					switch (status)
					{
					case PCDM_SS_OK:
					{
						int size = (int)output.str().length();
						auto buffer = std::make_unique<char[]>(size);
						output.seekg(0);
						output.read(buffer.get(), size);
						cli::array<System::Byte>^ byteArray = gcnew cli::array<System::Byte>(size);
						System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)buffer.get(), byteArray, 0, size);
						return byteArray;
					}
					case PCDM_SS_DriverFailure:
						throw gcnew XbimGeometryServiceException("Geometry save error, Driver Failure");
						break;
					case PCDM_SS_WriteFailure:
						throw gcnew XbimGeometryServiceException("Geometry save error, Write Failure");
						break;
					case PCDM_SS_Failure:
						throw gcnew XbimGeometryServiceException("Geometry save error, General Failure");
						break;
					case PCDM_SS_Doc_IsNull:
						throw gcnew XbimGeometryServiceException("Geometry save error, Document is NULL");
						break;
					case PCDM_SS_No_Obj:
						return nullptr; //empty store just return null			
					case PCDM_SS_Info_Section_Error:
						throw gcnew XbimGeometryServiceException("Geometry save error, Info Section Error");
						break;
					case PCDM_SS_UserBreak:
						throw gcnew XbimGeometryServiceException("Geometry save error, User Break");
						break;
					default:
						throw gcnew XbimGeometryServiceException("Geometry save error, Unknown Failure");
						break;
					}
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Geometry save error, " + msg);
				}
				return nullptr;
			}


			void BRepDocumentManager::Close(IXBRepDocument^ document)
			{
				BRepDocument^ doc = dynamic_cast<BRepDocument^>(document);
				if (doc == nullptr) //this is a programmatic error
					throw gcnew System::Exception("Document  cannot be converted to Xbim BRepDocument");
				try
				{
					Ref()->Close(doc->Ref());
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					_loggingService->LogError("Save Document Failed: " + msg);
					throw gcnew System::Exception("Document Manager cannot close document");
				}

			}

			

			array<System::Byte>^ BRepDocumentManager::ToArray(IXShape^ shape, bool withTriangles, bool withNormals)
			{
				try
				{
					if (shape == nullptr) return  gcnew cli::array<System::Byte>(0);
					TopoDS_Shape topoShape = ((XShape^)shape)->GetTopoShape();
					std::stringstream output;
					BinTools::Write(topoShape, output, withTriangles, withNormals, BinTools_FormatVersion::BinTools_FormatVersion_VERSION_3); //version is fixed at 3, the most current at time of authoring
					int size = (int)output.str().length();
					
					auto buffer = std::make_unique<char[]>(size);
					output.seekg(0);
					output.read(buffer.get(), size);
					cli::array<System::Byte>^ byteArray = gcnew cli::array<System::Byte>(size);
					System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)buffer.get(), byteArray, 0, size);
					return byteArray;
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Shape save error, " + msg);
				}
			}

			IXShape^ BRepDocumentManager::FromArray(array<System::Byte>^ bytes)
			{
				TopoDS_Shape topoShape;
				
				try
				{
					
					pin_ptr<System::Byte> pbArr = &bytes[0];				
					memstream iStrm(pbArr, bytes->Length);
					BinTools::Read(topoShape, iStrm);
					return XShape::GetXbimShape(topoShape);
				}
				catch (const Standard_Failure& sf)
				{
					std::stringstream strm;
					sf.Print(strm);
					System::String^ msg = gcnew System::String(strm.str().c_str());
					throw gcnew System::Exception("Shape save error, " + msg);
				}
			
				
			}


		}
		
	}
}
