#pragma once
#pragma warning( disable : 4691 )
#include "Unmanaged/FlexDrivers.h"
#include "../XbimHandle.h"
#include "Unmanaged//FlexApp_Application.h"
#include "../Services//LoggingService.h"
#include "../Services/ModelGeometryService.h"
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Services;

namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{
			public ref class BRepDocumentManager : XbimHandle<Handle(FlexApp_Application)>, IXBRepDocumentManager
			{
			private:
				IXLoggingService^ _loggingService;
				ModelGeometryService^ _modelService;
				Object^ _lockObject = gcnew Object();
			public:
				BRepDocumentManager(ModelGeometryService^ modelService) :
					XbimHandle(new Handle(FlexApp_Application)(FlexApp_Application::GetApplication())) //ensure only one application per session

				{
					_modelService = modelService;
					_loggingService = modelService->LoggingService;
					FlexDrivers::DefineFormat(Ref()); //initialise the application drivers
				};
				/// <summary>
				/// Returns true if the document manager is fully loaded
				/// </summary>
				property bool IsLoaded { virtual bool get() { return true;/* Ref()->IsDriverLoaded();*/ } }
				/// <summary>
				/// Number of documents open in the current session
				/// </summary>
				property int NbDocumentsInSession {virtual int get() { return true; /* Ref()->NbDocuments();*/ } }
				/// <summary>
				/// True if the named document is open in the current session
				/// </summary>
				/// <param name="filePath"></param>
				/// <returns></returns>
				virtual int InSession(System::String^ filePath);
				/// <summary>
				/// Returns the in session document
				/// </summary>
				/// <param name="docIndex"></param>
				/// <returns></returns>
				virtual IXBRepDocument^ GetDocument(int docIndex);
				/// <summary>
				/// Returns the in session index of document, use GetDocument to access the Storage Document
				/// </summary>
				/// <param name="filePath"></param>
				/// <returns></returns>
				virtual IXBRepDocument^ Open(System::String^ filePath);
				virtual IXBRepDocument^ Open(array<System::Byte>^ bytes);
				/// <summary>
				/// Returns the in session index of the newly created document, use GetDocument to access the Storage Document
				/// </summary>
				/// <returns></returns>
				virtual IXBRepDocument^ NewDocument();
				virtual IXBRepDocument^ NewDocument(double oneMeter, double precision);
				//virtual IXMaterialisedShapeDocument^ NewDocument(int ifcId, short ifcTypeId, int ifcShapeId, IXShape^ shape, IXVisualMaterial^ material, double oneMeter);

				/// <summary>
				/// Save the document
				/// </summary>
				/// <returns></returns>
				virtual bool Save(IXBRepDocument^ document);
				/// <summary>
				/// 
				/// </summary>
				/// <param name="filePath"></param>
				/// <param name="document"></param>
				/// <returns></returns>
				virtual bool SaveAs(System::String^ filePath, IXBRepDocument^ document);
				/// <summary>
				/// Returns the document as a byte array
				/// </summary>
				/// <param name="document"></param>
				/// <returns></returns>
				virtual array<System::Byte>^ ToArray(IXBRepDocument^ document);
				/// <summary>
				/// Close the storage document and remove from the session
				/// </summary>
				/// <param name="document"></param>
				virtual void Close(IXBRepDocument^ document);	

				virtual array<System::Byte>^ ToArray(IXShape^ shape, bool withTriangles, bool withNormals);
				virtual IXShape^ FromArray(array<System::Byte>^ bytes);
			};
		}
	}
}

