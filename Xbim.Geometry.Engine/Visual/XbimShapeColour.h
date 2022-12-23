#pragma once	

#include <XCAFDoc_VisMaterial.hxx>
#include <XCAFDoc_VisMaterialCommon.hxx>
#include "ColourRGB.h"
#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>
#include <TDataStd_Name.hxx>
#include "../XbimHandle.h"

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
			public ref class XbimShapeColour : XbimHandle<XCAFDoc_VisMaterialCommon>, IXShapeColour
			{
			private:

				System::String^ _name;
				int _id;
			public:
				XbimShapeColour(System::String^ colour, IIfcSurfaceStyleElementSelect^ ifcSurfaceStyleElementSelect) :XbimHandle(new XCAFDoc_VisMaterialCommon())
				{
					_name = colour;
					SetStyle(ifcSurfaceStyleElementSelect);
				}

				XbimShapeColour(int id, System::String^ colour) : XbimHandle(new XCAFDoc_VisMaterialCommon())
				{
					_id = id;
					_name = colour;
					Shininess = 1.0f;
					Transparency = 0.0f;
				}
				XbimShapeColour(const XCAFDoc_VisMaterialCommon& visMat) : XbimHandle(new XCAFDoc_VisMaterialCommon(visMat))
				{

				};
				virtual property int Id {int get() { return _id; }; void set(int id) { _id = id; }; }
				
				virtual property System::String^ Name {System::String^ get() { return _name; }; void set(System::String^ value) { _name = value; }; }
				virtual property IXColourRGB^ AmbientColor { IXColourRGB^ get();  void set(IXColourRGB^ colour); }
				virtual property IXColourRGB^ DiffuseColor { IXColourRGB^ get(); void set(IXColourRGB^ colour); }
				virtual property IXColourRGB^ SpecularColor { IXColourRGB^ get(); void set(IXColourRGB^ colour); }
				virtual property IXColourRGB^ EmissiveColor { IXColourRGB^ get(); void set(IXColourRGB^ colour); }
				virtual property float Shininess { float get(); void set(float shininess); }
				virtual property float Transparency { float get(); void set(float transparency); }
				
				virtual property NodeType NodeType {
					Xbim::Geometry::Abstractions::NodeType get() { return Xbim::Geometry::Abstractions::NodeType::Colour; };
					void set(Xbim::Geometry::Abstractions::NodeType nodeType) { };
				}
			
		private:
			void SetStyle(IIfcSurfaceStyleElementSelect^ ifcSurfaceStyleElementSelect);
		};
	}
}
}
