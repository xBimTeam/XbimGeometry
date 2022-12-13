#pragma once	

#include <TDF_Label.hxx>
#include <TDF_Tool.hxx>

#include "../XbimHandle.h"
#include "BRepDocument.h"
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Runtime::InteropServices;

using namespace System::Collections::Generic;
using namespace Microsoft::Extensions::Logging;

#define The_ShapeTool() XCAFDoc_DocumentTool::ShapeTool(Ref())
namespace Xbim
{
	namespace Geometry
	{
		namespace Storage
		{
			public ref class BRepDocumentItem : XbimHandle<TDF_Label>, IXBRepDocumentItem
			{
			private:
				
			public:
				static void SetName(const TDF_Label& label, System::String^ name);
				
				BRepDocumentItem(TDF_Label label) : XbimHandle(new TDF_Label(label))
				{		
				};

				BRepDocumentItem() : XbimHandle(new TDF_Label())
				{
				};

				virtual property IXShape^ Shape {IXShape^ get(); void set(IXShape^ shape); }
				virtual property System::String^ Key {System::String^ get(); }
				virtual property System::String^ Name {System::String^ get(); void set(System::String^ name); }
				virtual property bool IsStored {bool get() { return !Ref().IsNull(); }; };
				virtual property bool IsTopLevel {bool get() { return The_ShapeTool()->IsTopLevel(Ref()); }; };
				virtual property bool IsShape {bool get() { return The_ShapeTool()->IsShape(Ref()); }; };

				virtual property bool IsSubShape {bool get() { return The_ShapeTool()->IsSubShape(Ref()); }; };
				virtual property bool IsFree {bool get() { return The_ShapeTool()->IsFree(Ref()); }; };
				virtual property bool IsAssembly {bool get() { return The_ShapeTool()->IsAssembly(Ref()); }; };
				virtual property bool IsComponent {bool get() { return The_ShapeTool()->IsComponent(Ref()); }; };
				virtual property bool IsCompound {bool get() { return The_ShapeTool()->IsCompound(Ref()); }; };
				virtual property bool IsExternalRef {bool get() { return The_ShapeTool()->IsExternRef(Ref()); }; };
				virtual property bool IsSimpleShape {bool get() { return The_ShapeTool()->IsSimpleShape(Ref()); }; };
				virtual property bool IsReference {bool get() { return The_ShapeTool()->IsReference(Ref()); }; };
				virtual property bool IsNull {bool get() { return Ref().IsNull(); }; };
				virtual property System::Nullable<double> Volume {System::Nullable<double> get(); void set(System::Nullable<double> volume); }
				virtual property System::Nullable<double> Area {System::Nullable<double> get(); void set(System::Nullable<double> area); }
				virtual property System::Nullable<double> HeightMax {System::Nullable<double> get(); void set(System::Nullable<double> height); }
				virtual property System::Nullable<double> HeightMin {System::Nullable<double> get(); void set(System::Nullable<double> height); }
				virtual property System::Nullable<double> ThicknessMax {System::Nullable<double> get(); void set(System::Nullable<double> thickness); }
				virtual property int NbMaterialLayers {int get(); void set(int nbMaterialLayers); }
				virtual void AddReference(IXBRepDocumentItem^ referred, System::Nullable<XbimMatrix3D> transform);
				virtual IXBRepDocumentItem^ AddShape(System::String^ name, IXShape^ shape, bool expand);
				virtual void SetPlacement(IIfcObjectPlacement^ placement);
				virtual IXBRepDocumentItem^ AddAssembly(System::String^ subAssemblyName);
				virtual IXBRepDocumentItem^ AddAssembly(System::String^ subAssemblyName, IIfcObjectPlacement^ placement);
				virtual IXBRepDocumentItem^ AddAssembly(System::String^ subAssemblyName, XbimMatrix3D transform);

				virtual IXBRepDocumentItem^ AddComponent(System::String^ name, IXBRepDocumentItem^ component, IIfcObjectPlacement^ objPlacement, ILogger^ logger);

				virtual IXBRepDocumentItem^ AddComponent(System::String^ name, IXBRepDocumentItem^ componentItem, XbimMatrix3D transform);

				virtual IXBRepDocumentItem^ AddComponent(System::String^ name, int shapeId, IXShape^ shape, IIfcObjectPlacement^ objPlacement, ILogger^ logger);

				virtual IXBRepDocumentItem^ AddComponent(System::String^ name, int shapeId, IXShape^ shape);
				virtual IXBRepDocumentItem^ AddSubShape(System::String^ name, IXShape^ shape, bool isExistingPart);


				virtual System::String^ GetStringAttribute(System::String^ attributeName);

				virtual void SetStringAttribute(System::String^ attributeName, System::String^ attributeValue);

				virtual int GetIntAttribute(System::String^ attributeName);

				virtual void SetIntAttribute(System::String^ attributeName, int attributeValue);
				virtual System::Nullable<double> GetDoubleAttribute(System::String^ attributeName);

				virtual void SetDoubleAttribute(System::String^ attributeName, System::Nullable<double> attributeValue);
				TopoDS_Shape GetShape();

				void SetShape(const TopoDS_Shape shape);

				virtual property int NbComponents {int get(); }
				virtual property IEnumerable<IXBRepDocumentItem^>^ Components {IEnumerable<IXBRepDocumentItem^>^ get(); }
				virtual property IEnumerable<IXBRepDocumentItem^>^ SubShapeStorageItems {IEnumerable<IXBRepDocumentItem^>^ get(); }
				virtual property IEnumerable<IXShape^>^ SubShapes {IEnumerable<IXShape^>^ get(); }
				virtual property IXBRepDocumentItem^ ReferredShape {IXBRepDocumentItem^ get(); }
				
				virtual property int IfcId { int get(); void set(int id); }
				virtual property short IfcTypeId { short get(); void set(short id); }
				virtual property System::String^ IfcType { System::String^ get(); void set(System::String^ typeName); }
				virtual property System::String^ Classification { System::String^ get(); void set(System::String^ classification); }
				virtual property System::String^ IfcName { System::String^ get(); void set(System::String^ name); }
				virtual property bool HasOpenings { bool get(); void set(bool hasOpenings); }
				virtual property bool HasProjections { bool get(); void set(bool hasProjections); }
				virtual property bool IsProduct { bool get(); void set(bool hasOpenings); }
				virtual property bool IsShapeRepresentation { bool get(); void set(bool isShapeRep); }
				virtual property bool IsShapeRepresentationMap { bool get(); void set(bool isShapeRepMap); }
				virtual property bool IsGeometricRepresentationItem { bool get(); void set(bool isGeomRep); }
				
				
			};

		}
	}
}