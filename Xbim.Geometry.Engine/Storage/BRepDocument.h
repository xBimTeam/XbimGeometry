#pragma once	


#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDocStd_Application.hxx>


#include "Unmanaged/FlexApp_Application.h"
//#include "Unmanaged/FlexDrivers.h"

using namespace Xbim::Common::Geometry;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;


using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace System::IO;


#include "../XbimHandle.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{


			public ref class BRepDocument : XbimHandle<Handle(TDocStd_Document)>, IXBRepDocument
			{
			

			private:

			public:

				BRepDocument(Handle(TDocStd_Document) hDoc) : XbimHandle(new Handle(TDocStd_Document)(hDoc))
				{

				};
				///Tidy up the document make sure it is closed aand removed from the in session documents on dispose
				~BRepDocument()
				{
					if (FlexApp_Application::GetApplication()->CanClose(Ref()) == CDM_CanCloseStatus::CDM_CCS_OK)
						FlexApp_Application::GetApplication()->Close(Ref());
				}

				virtual IXBRepDocumentItem^ CreateAssembly(System::String^ name);
				virtual IXBRepDocumentItem^ CreateAssembly(System::String^ name, IXShape^ assemblyShape);

				
				virtual IXBRepDocumentItem^ AddShape(int id, IXShape^ shape);
				virtual IXShape^ GetShape(int id);

				
				virtual bool RemoveAssembly(IXBRepDocumentItem^ assembly);
				bool RemoveComponents(const TDF_Label& aShapeLabel);
				
				virtual property IXBRepDocumentItem^ RootItem {IXBRepDocumentItem^ get(); };
				virtual property IEnumerable<IXBRepDocumentItem^>^ FreeShapes {IEnumerable<IXBRepDocumentItem^>^ get(); };
				virtual property IEnumerable<IXBRepDocumentItem^>^ Shapes {IEnumerable<IXBRepDocumentItem^>^ get(); };
				virtual property System::Nullable<double> ConversionFactorForOneMeter {
					System::Nullable<double> get() { return RootItem->GetDoubleAttribute("ConversionFactorForOneMeter"); };
					void set(System::Nullable<double> val) { RootItem->SetDoubleAttribute("ConversionFactorForOneMeter", val); }; };
				virtual property System::Nullable<double> PrecisionFactor {
					System::Nullable<double> get() { return RootItem->GetDoubleAttribute("PrecisionFactor"); };
					void set(System::Nullable<double> val) { RootItem->SetDoubleAttribute("PrecisionFactor", val); }; };
				virtual void UpdateAssemblies();
				virtual IXVisualMaterial^ AddVisualMaterial(IXVisualMaterial^ visMaterial);
				virtual void SetMaterial(IXShape^ shape, IXVisualMaterial^ visMaterial);
				virtual IEnumerable<IXVisualMaterial^>^ GetVisualMaterials();
				virtual IEnumerable<IXBRepDocumentItem^>^ GetMaterials();
				virtual IXVisualMaterial^ GetMaterial(IXBRepDocumentItem^ item);
				virtual IXVisualMaterial^ GetMaterial(IXShape^ shape);


			};
		}
	}
}