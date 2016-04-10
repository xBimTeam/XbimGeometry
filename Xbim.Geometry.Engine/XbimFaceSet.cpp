#include "XbimFaceSet.h"
#include "XbimConvert.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
#pragma region Constructors

		XbimFaceSet::XbimFaceSet(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_FACE, map);
			faces = gcnew  List<IXbimFace^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				faces->Add(gcnew XbimFace(TopoDS::Face(map(i))));
		}

		XbimFaceSet::XbimFaceSet(const TopTools_ListOfShape & shapes)
		{
			faces = gcnew  List<IXbimFace^>(shapes.Extent());
			for (TopTools_ListIteratorOfListOfShape faceIter(shapes); faceIter.More(); faceIter.Next())
				faces->Add(gcnew XbimFace(TopoDS::Face(faceIter.Value())));
		}

		XbimFaceSet::XbimFaceSet(List<IXbimFace^>^ faces)
		{
			this->faces = faces;
		}
#pragma endregion


		IXbimFace^ XbimFaceSet::First::get()
		{
			if (faces->Count == 0) return nullptr;
			return faces[0];
		}

		int XbimFaceSet::Count::get()
		{
			return faces == nullptr ? 0 : faces->Count;
		}

		IXbimGeometryObject^ XbimFaceSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimFace^>^ result = gcnew List<IXbimFace^>(faces->Count);
			for each (IXbimGeometryObject^ face in faces)
			{
				result->Add((IXbimFace^)face->Transform(matrix3D));
			}
			return gcnew XbimFaceSet(result);
		}

		IXbimGeometryObject^ XbimFaceSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimFace^>^ result = gcnew List<IXbimFace^>(faces->Count);
			for each (IXbimGeometryObject^ face in faces)
			{
				result->Add((IXbimFace^)((XbimFace^)face)->TransformShallow(matrix3D));
			}
			return gcnew XbimFaceSet(result);
		}

		IXbimGeometryObject ^ XbimFaceSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimFaceSet^ result = gcnew XbimFaceSet();
			for each (XbimFace^ face in faces)
				result->Add((XbimFace^)face->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimFaceSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimFaceSet^ result = gcnew XbimFaceSet();
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimFace^ face in faces)
			{
				XbimFace^ copy = gcnew XbimFace((XbimFace^)face, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimFaceSet::Moved(IIfcObjectPlacement ^ objectPlacement)
		{
			if (!IsValid) return this;
			XbimFaceSet^ result = gcnew XbimFaceSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement);
			for each (IXbimFace^ face in faces)
			{
				XbimFace^ copy = gcnew XbimFace((XbimFace^)face, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimFaceSet::Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimFace^ face  in faces)
			{
				((XbimFace^)face)->Mesh(mesh, precision, deflection, angle);
			}
		}


		XbimRect3D XbimFaceSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in faces)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IEnumerator<IXbimFace^>^ XbimFaceSet::GetEnumerator()
		{
			return faces->GetEnumerator();
		}
	}
}
