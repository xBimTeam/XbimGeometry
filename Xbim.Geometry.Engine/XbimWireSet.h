#pragma once
#include "XbimWire.h"
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
using namespace XbimGeometry::Interfaces;
using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		ref class XbimWireSet : IXbimWireSet
		{
		private:
			List<IXbimWire^>^ wires;
			static XbimWireSet^ empty = gcnew XbimWireSet();
			XbimWireSet::XbimWireSet(){ wires = gcnew List<IXbimWire^>(1); }
		public:
			static property XbimWireSet^ Empty{XbimWireSet^ get(){ return empty; }};
#pragma region Constructors
			XbimWireSet(const TopoDS_Shape& shape);
			XbimWireSet(const TopTools_ListOfShape & wires);
			XbimWireSet(IEnumerable<IXbimWire^>^ wires){ this->wires = gcnew List<IXbimWire^>(wires); };
#pragma endregion

			virtual property bool IsValid{bool get(){ return Count>0; }; }
			virtual property bool IsSet{bool get()  { return true; }; }
			virtual property IXbimWire^ First{IXbimWire^ get(); }
			virtual property int Count{int get(); }
			virtual property XbimRect3D BoundingBox {XbimRect3D get() ; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimWireSetType; }}
			virtual IEnumerator<IXbimWire^>^ GetEnumerator();
			virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator{ return GetEnumerator(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D);
		};

	}
}

