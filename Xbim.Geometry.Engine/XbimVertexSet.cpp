
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include "XbimVertexSet.h"
#include "XbimConvert.h"
#include <BRep_Builder.hxx>

#include "./BRep/XCompound.h"
#include "XbimGeometryObjectSet.h"
namespace Xbim
{
	namespace Geometry
	{
		IXCompound^ XbimVertexSet::ToXCompound()
		{
			return gcnew Xbim::Geometry::BRep::XCompound(XbimGeometryObjectSet::CreateCompound(Enumerable::Cast<IXbimGeometryObject^>(vertices)));
		}
		XbimVertexSet::operator  TopoDS_Shape ()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimVertex ^ vertex in vertices)
			{
				builder.Add(bodyCompound, (XbimVertex^) vertex);
			}
			return bodyCompound;
		}
#pragma region Constructors
		


		XbimVertexSet::XbimVertexSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_VERTEX, map);
			vertices = gcnew  List<IXbimVertex^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				vertices->Add(gcnew XbimVertex(TopoDS::Vertex(map(i))));
		}

		XbimVertexSet::XbimVertexSet(IEnumerable<IXbimVertex^>^ vertices, ModelGeometryService^ modelService) : XbimSetObject(modelService)
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
			return gcnew XbimVertexSet(result, _modelServices);
		}

		IXbimGeometryObject^ XbimVertexSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimVertex^>^ result = gcnew List<IXbimVertex^>(vertices->Count);
			for each (IXbimGeometryObject^ vertex in vertices)
			{
				result->Add((IXbimVertex^)((XbimVertex^)vertex)->TransformShallow(matrix3D));
			}
			return gcnew XbimVertexSet(result, _modelServices);
		}

		IXbimGeometryObject ^ XbimVertexSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimVertexSet^ result = gcnew XbimVertexSet(_modelServices);
			for each (XbimVertex^ vertex in vertices)
				result->Add((XbimVertex^)vertex->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimVertexSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimVertexSet^ result = gcnew XbimVertexSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimVertex^ vertex in vertices)
			{
				XbimVertex^ copy = gcnew XbimVertex((XbimVertex^)vertex, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimVertexSet::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger )
		{
			if (!IsValid) return this;
			XbimVertexSet^ result = gcnew XbimVertexSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger, _modelServices);
			for each (IXbimVertex^ vertex in vertices)
			{
				XbimVertex^ copy = gcnew XbimVertex((XbimVertex^)vertex, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimVertexSet::Mesh(IXbimMeshReceiver ^ /*mesh*/, double /*precision*/, double /*deflection*/, double /*angle*/)
		{
			throw gcnew System::NotImplementedException("XbimVertexSet::Mesh");
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
