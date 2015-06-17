#include "XbimFaceSet.h"
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
