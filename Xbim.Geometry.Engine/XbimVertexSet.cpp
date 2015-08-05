#include "XbimVertexSet.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>

#pragma region Carve includes

#pragma endregion

using namespace System;
namespace Xbim
{
	namespace Geometry
	{
#pragma region Constructors
		

		XbimVertexSet::XbimVertexSet(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_VERTEX, map);
			vertices = gcnew  List<IXbimVertex^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				vertices->Add(gcnew XbimVertex(TopoDS::Vertex(map(i))));
		}

		XbimVertexSet::XbimVertexSet(IEnumerable<IXbimVertex^>^ vertices)
		{
			this->vertices = gcnew List<IXbimVertex^>(vertices);
		}

#pragma endregion


		IXbimVertex^ XbimVertexSet::First::get()
		{
			if (vertices->Count == 0) return nullptr;
			return vertices[0];
		}

		int XbimVertexSet::Count::get()
		{
			return vertices == nullptr ? 0 : vertices->Count;
		}

		IXbimGeometryObject^ XbimVertexSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimVertex^>^ result = gcnew List<IXbimVertex^>(vertices->Count);
			for each (IXbimGeometryObject^ vertex in vertices)
			{
				result->Add((IXbimVertex^)vertex->Transform(matrix3D));
			}
			return gcnew XbimVertexSet(result);
		}

		IXbimGeometryObject^ XbimVertexSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimVertex^>^ result = gcnew List<IXbimVertex^>(vertices->Count);
			for each (IXbimGeometryObject^ vertex in vertices)
			{
				result->Add((IXbimVertex^)((XbimVertex^)vertex)->TransformShallow(matrix3D));
			}
			return gcnew XbimVertexSet(result);
		}


		XbimRect3D XbimVertexSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in vertices)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IEnumerator<IXbimVertex^>^ XbimVertexSet::GetEnumerator()
		{
			return vertices->GetEnumerator();
		}
	}
}
