
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRep_Builder.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Array1OfBox.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include "XbimShellSet.h"
#include "XbimSolid.h"
#include "XbimSolidSet.h"

#include "XbimGeometryObjectSet.h"
#include "XbimConvert.h"
#include "./BRep/XCompound.h"
#include "./Factories/BooleanFactory.h"
namespace Xbim
{
	namespace Geometry
	{
		IXCompound^ XbimShellSet::ToXCompound()
		{
			return gcnew Xbim::Geometry::BRep::XCompound(XbimGeometryObjectSet::CreateCompound(Enumerable::Cast<IXbimGeometryObject^>(shells)));
		}

		XbimShellSet::XbimShellSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService) :XbimSetObject(modelService)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_SHELL, map);
			shells = gcnew  List<IXbimShell^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				shells->Add(gcnew XbimShell(TopoDS::Shell(map(i)), _modelServices));
		}

		XbimShellSet::XbimShellSet(List<IXbimShell^>^ shells, ModelGeometryService^ modelService) :XbimSetObject(modelService)
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
			for each (IXbimGeometryObject ^ shell in shells)
			{
				result->Add((IXbimShell^)shell->Transform(matrix3D));
			}
			return gcnew XbimShellSet(result, _modelServices);
		}

		IXbimGeometryObject^ XbimShellSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimShell^>^ result = gcnew List<IXbimShell^>(shells->Count);
			for each (IXbimGeometryObject ^ shell in shells)
			{
				result->Add((IXbimShell^)((XbimShell^)shell)->TransformShallow(matrix3D));
			}
			return gcnew XbimShellSet(result, _modelServices);
		}

		XbimRect3D XbimShellSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject ^ geomObj in shells)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		System::Collections::Generic::IEnumerator<IXbimShell^>^ XbimShellSet::GetEnumerator()
		{
			return shells->GetEnumerator();
		}

		void XbimShellSet::Add(IXbimGeometryObject^ shape)
		{
			IXbimShell^ shell = dynamic_cast<IXbimShell^>(shape);
			if (shell != nullptr) return shells->Add(shell);
			IXbimShellSet^ shellSet = dynamic_cast<IXbimShellSet^>(shape);
			if (shellSet != nullptr) return shells->AddRange(shellSet);
			IXbimGeometryObjectSet^ geomSet = dynamic_cast<IXbimGeometryObjectSet^>(shape);
			if (geomSet != nullptr)  return shells->AddRange(geomSet->Shells);
			IXbimSolid^ solid = dynamic_cast<IXbimSolid^>(shape);
			if (solid != nullptr) return shells->AddRange(solid->Shells);
		}

		bool XbimShellSet::IsPolyhedron::get()
		{
			for each (IXbimShell ^ shell in shells)
			{
				if (!shell->IsPolyhedron) return false;
			}
			return true;
		}



		IXbimGeometryObject^ XbimShellSet::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			if (!IsValid) return this;
			XbimShellSet^ result = gcnew XbimShellSet(_modelServices);
			for each (XbimShell ^ shell in shells)
				result->Add((XbimShell^)shell->Transformed(transformation));
			return result;
		}

		void XbimShellSet::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			for each (IXbimShell ^ shell in shells)
			{
				((XbimShell^)shell)->Move(position);
			}
		}

		IXbimGeometryObject^ XbimShellSet::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimShellSet^ result = gcnew XbimShellSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimShell ^ shell in shells)
			{
				XbimShell^ copy = gcnew XbimShell((XbimShell^)shell, Tag, _modelServices);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject^ XbimShellSet::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimShellSet^ result = gcnew XbimShellSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger, _modelServices);
			for each (IXbimShell ^ shell in shells)
			{
				XbimShell^ copy = gcnew XbimShell((XbimShell^)shell, Tag, _modelServices);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimShellSet::Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimShell ^ shell  in shells)
			{
				((XbimShell^)shell)->Mesh(mesh, precision, deflection, angle);
			}
		}

		XbimShellSet::operator TopoDS_Shape()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimShell ^ shell in shells)
			{
				builder.Add(bodyCompound, (XbimShell^)shell);
			}
			return bodyCompound;
		}

		IXbimGeometryObjectSet^ XbimShellSet::Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto shell in shells)
				tools.Append(static_cast<XbimShell^>(shell));
			for each (auto solid in solids)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShellSet::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto shell in shells)
				tools.Append(static_cast<XbimShell^>(shell));
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}


		IXbimGeometryObjectSet^ XbimShellSet::Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto shell in shells)
				tools.Append(static_cast<XbimShell^>(shell));
			for each (auto solid in solids)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShellSet::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto shell in shells)
				tools.Append(static_cast<XbimShell^>(shell));
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShellSet::Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto shell in shells)
				tools.Append(static_cast<XbimShell^>(shell));
			for each (auto solid in solids)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShellSet::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto shell in shells)
				tools.Append(static_cast<XbimShell^>(shell));
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}

		void XbimShellSet::Union(double tolerance)
		{
			if (Count > 1)
			{
				TopoDS_ListOfShape arguments;
				TopoDS_ListOfShape tools;
				arguments.Append(static_cast<XbimShell^>(Enumerable::First(shells)));
				for each (auto shell in Enumerable::Skip(shells, 1))
					tools.Append(static_cast<XbimShell^>(shell));
				TopoDS_Shape result;
				if (tolerance > 0)
					result = gcnew XbimShellSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
				else
					result = gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(result, TopAbs_SHELL, map);
				shells = gcnew  List<IXbimShell^>(map.Extent());
				for (int i = 1; i <= map.Extent(); i++)
					shells->Add(gcnew XbimShell(TopoDS::Shell(map(i)), _modelServices));
			}
		}
	}
}
