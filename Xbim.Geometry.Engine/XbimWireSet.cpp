
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include "XbimWireSet.h"
#include "XbimConvert.h"
#include <BRep_Builder.hxx>
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
				wires->Add(gcnew XbimWireV5(TopoDS::Wire(map(i))));
		}

		XbimWireSet::XbimWireSet(const TopTools_ListOfShape& wireList)
		{
			wires = gcnew  List<IXbimWire^>(wireList.Extent());
			for (TopTools_ListIteratorOfListOfShape wireIter(wireList); wireIter.More(); wireIter.Next())
				wires->Add(gcnew XbimWireV5(TopoDS::Wire(wireIter.Value())));
		}

		XbimWireSet::operator TopoDS_Shape ()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (XbimWireV5^ wire in wires)
			{				
				builder.Add(bodyCompound, wire);	
			}
			return bodyCompound;
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

		IXbimGeometryObject^ XbimWireSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimWire^>^ result = gcnew List<IXbimWire^>(wires->Count);
			for each (IXbimGeometryObject^ wire in wires)
			{
				result->Add((IXbimWire^)((XbimWireV5^)wire)->TransformShallow(matrix3D));
			}
			return gcnew XbimWireSet(result);
		}

		IXbimGeometryObject ^ XbimWireSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimWireSet^ result = gcnew XbimWireSet();
			for each (XbimWireV5^ wire in wires)
				result->Add((XbimWireV5^)wire->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimWireSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimWireSet^ result = gcnew XbimWireSet();
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimFace^ wire in wires)
			{
				XbimWireV5^ copy = gcnew XbimWireV5((XbimWireV5^)wire, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimWireSet::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimWireSet^ result = gcnew XbimWireSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement,logger);
			for each (IXbimFace^ wire in wires)
			{
				XbimWireV5^ copy = gcnew XbimWireV5((XbimWireV5^)wire, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimWireSet::Mesh(IXbimMeshReceiver ^ /*mesh*/, double /*precision*/, double /*deflection*/, double /*angle*/)
		{
			throw gcnew System::NotImplementedException("XbimWireSet::Mesh");
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
