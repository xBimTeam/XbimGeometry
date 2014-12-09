#include "XbimShellSet.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		XbimShellSet::XbimShellSet(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_SHELL, map);
			shells = gcnew  List<IXbimShell^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				shells->Add(gcnew XbimShell(TopoDS::Shell(map(i))));
		}

		XbimShellSet::XbimShellSet(List<IXbimShell^>^ shells)
		{
			this->shells = shells;
		}


		IXbimShell^ XbimShellSet::First::get()
		{
			if (shells->Count == 0) return nullptr;
			return shells[0];
		}

		int XbimShellSet::Count::get()
		{
			return shells == nullptr ? 0 : shells->Count;
		}

		IXbimGeometryObject^ XbimShellSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimShell^>^ result = gcnew List<IXbimShell^>(shells->Count);
			for each (IXbimGeometryObject^ shell in shells)
			{
				result->Add((IXbimShell^)shell->Transform(matrix3D));
			}
			return gcnew XbimShellSet(result);
		}

		XbimRect3D XbimShellSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in shells)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IEnumerator<IXbimShell^>^ XbimShellSet::GetEnumerator()
		{
			return shells->GetEnumerator();
		}
	}
}
