#pragma once	

#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDocStd_Application.hxx>

#include "Unmanaged/FlexApp_Application.h"
#include "Unmanaged/FlexDrivers.h"

using namespace Xbim::Common::Geometry;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;


using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace System::IO;

#define XCAFDoc_ShapeTool() XCAFDoc_DocumentTool::ShapeTool(Ref()->Main())
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

				virtual IXStorageItem^ CreateAssembly(System::String^ name);
				virtual IXStorageItem^ CreateAssembly(System::String^ name, IXShape^ assemblyShape);

				virtual bool ExportGltf(System::String^ fileName, bool binary);
				virtual bool ExportSTEP(System::String^ fileName);
				virtual array<System::Byte>^ ExportWexbim(IXColourRGB^ defaultColour, MeshGranularity meshGranularity);
				virtual bool ExportWexbim(System::String^ path, IXColourRGB^ defaultColour, MeshGranularity meshGranularity);
				void LoadSTEP(System::String^ path);
				virtual IXVisualMaterial^ AddVisualMaterial(IXVisualMaterial^ visMaterial);
				virtual IXStorageItem^ AddShape(int id, IXShape^ shape);
				virtual IXShape^ GetShape(int id);

				virtual void SetMaterial(IXShape^ shape, IXVisualMaterial^ visMaterial);

				virtual IEnumerable<IXVisualMaterial^>^ GetVisualMaterials();
				virtual IEnumerable<IXStorageItem^>^ GetMaterials();
				virtual bool RemoveAssembly(IXStorageItem^ assembly);
				bool RemoveComponents(const TDF_Label& aShapeLabel);
				virtual IXVisualMaterial^ GetMaterial(IXStorageItem^ item);
				virtual IXVisualMaterial^ GetMaterial(IXShape^ shape);
				virtual property IXStorageItem^ RootItem {IXStorageItem^ get(); };
				virtual property IEnumerable<IXStorageItem^>^ FreeShapes {IEnumerable<IXStorageItem^>^ get(); };
				virtual property IEnumerable<IXStorageItem^>^ Shapes {IEnumerable<IXStorageItem^>^ get(); };
				virtual property System::Nullable<double> ConversionFactorForOneMeter {
					System::Nullable<double> get() { return RootItem->GetDoubleAttribute("ConversionFactorForOneMeter"); };
					void set(System::Nullable<double> val) { RootItem->SetDoubleAttribute("ConversionFactorForOneMeter", val); }; };
				virtual property System::Nullable<double> PrecisionFactor {
					System::Nullable<double> get() { return RootItem->GetDoubleAttribute("PrecisionFactor"); };
					void set(System::Nullable<double> val) { RootItem->SetDoubleAttribute("PrecisionFactor", val); }; };
				virtual void UpdateAssemblies();


			};
		}
	}
}