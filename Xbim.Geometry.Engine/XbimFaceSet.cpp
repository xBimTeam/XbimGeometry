
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include "XbimFaceSet.h"
#include "XbimConvert.h"
#include <BRep_Builder.hxx>
#include "./BRep/XCompound.h"
#include "XbimGeometryObjectSet.h"
namespace Xbim
{
	namespace Geometry
	{
		IXCompound^ XbimFaceSet::ToXCompound()
		{
			return gcnew Xbim::Geometry::BRep::XCompound(XbimGeometryObjectSet::CreateCompound(Enumerable::Cast<IXbimGeometryObject^>(faces)));
		}

#pragma region Constructors

		XbimFaceSet::XbimFaceSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_FACE, map);
			faces = gcnew  List<IXbimFace^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				faces->Add(gcnew XbimFace(TopoDS::Face(map(i)), _modelServices));
		}

		XbimFaceSet::XbimFaceSet(const TopTools_ListOfShape & shapes, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			faces = gcnew  List<IXbimFace^>(shapes.Extent());
			for (TopTools_ListIteratorOfListOfShape faceIter(shapes); faceIter.More(); faceIter.Next())
				faces->Add(gcnew XbimFace(TopoDS::Face(faceIter.Value()), _modelServices));
		}

		XbimFaceSet::XbimFaceSet(List<IXbimFace^>^ faces, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			this->faces = faces;
		}
		XbimFaceSet::operator TopoDS_Shape()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimFace ^ face in faces)
			{
				builder.Add(bodyCompound, (XbimFace^)face);
			}
			return bodyCompound;
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
			return gcnew XbimFaceSet(result, _modelServices);
		}

		IXbimGeometryObject^ XbimFaceSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimFace^>^ result = gcnew List<IXbimFace^>(faces->Count);
			for each (IXbimGeometryObject^ face in faces)
			{
				result->Add((IXbimFace^)((XbimFace^)face)->TransformShallow(matrix3D));
			}
			return gcnew XbimFaceSet(result, _modelServices);
		}

		IXbimGeometryObject ^ XbimFaceSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimFaceSet^ result = gcnew XbimFaceSet(_modelServices);
			for each (XbimFace^ face in faces)
				result->Add((XbimFace^)face->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimFaceSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimFaceSet^ result = gcnew XbimFaceSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimFace^ face in faces)
			{
				XbimFace^ copy = gcnew XbimFace((const TopoDS_Face&)((XbimFace^)face), Tag, _modelServices);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimFaceSet::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimFaceSet^ result = gcnew XbimFaceSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement,logger, _modelServices);
			for each (IXbimFace^ face in faces)
			{
				XbimFace^ copy = gcnew XbimFace((const TopoDS_Face&)((XbimFace^)face), Tag, _modelServices);
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

		System::Collections::Generic::IEnumerator<IXbimFace^>^ XbimFaceSet::GetEnumerator()
		{
			return faces->GetEnumerator();
		}
	}
}
