#pragma once	
//This is included for future developments, but is not to be used in current release
#include <XCAFDoc_VisMaterial.hxx>
#include <XCAFDoc_VisMaterialCommon.hxx>
#include "ColourRGB.h"
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TDataStd_Name.hxx>
#include "../XbimHandle.h"
//#include "../storage/StorageItem.h"
using namespace Xbim::Common::Geometry;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;
using namespace Xbim::Ifc4::Interfaces;

namespace Xbim
{
	namespace Geometry
	{
		namespace Visual
		{
			public ref class VisualMaterial : XbimHandle<Handle(XCAFDoc_VisMaterial)>, IXVisualMaterial
			{
			private:
				/*StorageItem^ _label;*/
				System::String^ _name;
				
			public:
				VisualMaterial(System::String^ colour, IIfcSurfaceStyleElementSelect^ ifcSurfaceStyleElementSelect) :
					XbimHandle(new Handle(XCAFDoc_VisMaterial)(new XCAFDoc_VisMaterial()))
				{
					_name = colour;				
					SetStyle(ifcSurfaceStyleElementSelect);
				}

				VisualMaterial(System::String^ colour) : XbimHandle(new Handle(XCAFDoc_VisMaterial)(new XCAFDoc_VisMaterial()))
				{
					_name = colour;
					Shininess = 1.0f;
					Transparency = 0.0f;
				}
				VisualMaterial(Handle(XCAFDoc_VisMaterial) visMat/*, StorageItem^ label*/) : XbimHandle(new Handle(XCAFDoc_VisMaterial)(visMat))
				{
					//TCollection_AsciiString entry;
					//TDF_Tool::Entry(label->Ref(), entry);
					//_label = label;
					//TCollection_AsciiString aName;
					//Handle(TDataStd_Name) aNodeName;
					//if (label->Ref().FindAttribute(TDataStd_Name::GetID(), aNodeName))
					//{
					//	aName = aNodeName->Get(); // instance name
					//	_name = gcnew System::String(aName.ToCString());
					//}
				};
				virtual property IXStorageItem^ Label {IXStorageItem^ get() { throw gcnew System::NotImplementedException(); }; }
				virtual property System::String^ Name {System::String^ get() { return _name; }; }
				virtual property IXColourRGB^ AmbientColor { IXColourRGB^ get();  void set(IXColourRGB^ colour); }
				virtual property IXColourRGB^ DiffuseColor { IXColourRGB^ get(); void set(IXColourRGB^ colour); }
				virtual property IXColourRGB^ SpecularColor { IXColourRGB^ get(); void set(IXColourRGB^ colour); }
				virtual property IXColourRGB^ EmissiveColor { IXColourRGB^ get(); void set(IXColourRGB^ colour); }
				virtual property float Shininess { float get(); void set(float shininess); }
				virtual property float Transparency { float get(); void set(float transparency); }
				virtual property bool IsDefined {bool get(); }
				virtual property bool IsStored {bool get() { return false;/* _label != nullptr;*/ }; }
				virtual void SetPhysicalBasedRender();
			private:
				void SetStyle(IIfcSurfaceStyleElementSelect^ ifcSurfaceStyleElementSelect);
			};
		}
	}
}
