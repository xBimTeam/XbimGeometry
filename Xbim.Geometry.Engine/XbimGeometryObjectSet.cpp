#include "XbimGeometryObjectSet.h"

using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		XbimGeometryObjectSet::XbimGeometryObjectSet(IEnumerable<IXbimGeometryObject^>^ objects)
		{
			geometryObjects = gcnew List<IXbimGeometryObject^>(objects);
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::First::get()
		{
			if (geometryObjects->Count == 0) return nullptr;
			return geometryObjects[0];
		}

		int XbimGeometryObjectSet::Count::get()
		{
			return geometryObjects->Count;
		}

		IEnumerator<IXbimGeometryObject^>^ XbimGeometryObjectSet::GetEnumerator()
		{
			return geometryObjects->GetEnumerator();
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimGeometryObject^>^ result = gcnew List<IXbimGeometryObject^>(geometryObjects->Count);
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				result->Add(geomObj->Transform(matrix3D));
			}
			return gcnew XbimGeometryObjectSet(result);
		}

		XbimRect3D XbimGeometryObjectSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in geometryObjects)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}
	}
}
