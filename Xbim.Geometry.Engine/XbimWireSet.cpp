#include "XbimWireSet.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		XbimWireSet::XbimWireSet(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_WIRE, map);
			wires = gcnew  List<IXbimWire^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				wires->Add(gcnew XbimWire(TopoDS::Wire(map(i))));
		}

		XbimWireSet::XbimWireSet(const TopTools_ListOfShape& wireList)
		{
			wires = gcnew  List<IXbimWire^>(wireList.Extent());
			for (TopTools_ListIteratorOfListOfShape wireIter(wireList); wireIter.More(); wireIter.Next())
				wires->Add(gcnew XbimWire(TopoDS::Wire(wireIter.Value())));
		}

		IXbimWire^ XbimWireSet::First::get()
		{
			if (wires->Count == 0) return nullptr;
			return wires[0];
		}

		int XbimWireSet::Count::get()
		{
			return wires == nullptr ? 0 : wires->Count;
		}

		IXbimGeometryObject^ XbimWireSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimWire^>^ result = gcnew List<IXbimWire^>(wires->Count);
			for each (IXbimGeometryObject^ wire in wires)
			{
				result->Add((IXbimWire^)wire->Transform(matrix3D));
			}
			return gcnew XbimWireSet(result);
		}

		XbimRect3D XbimWireSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in wires)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IEnumerator<IXbimWire^>^ XbimWireSet::GetEnumerator()
		{
			return wires->GetEnumerator();
		}
	}
}
