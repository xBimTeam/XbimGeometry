#include "XbimShellSet.h"
#include "XbimSolid.h"
#include "XbimSolidSet.h"
#include "XbimGeometryCreator.h"
#include "XbimGeometryObjectSet.h"
#include "XbimConvert.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRep_Builder.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Array1OfBox.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
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

		IXbimGeometryObject^ XbimShellSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimShell^>^ result = gcnew List<IXbimShell^>(shells->Count);
			for each (IXbimGeometryObject^ shell in shells)
			{
				result->Add((IXbimShell^)((XbimShell^)shell)->TransformShallow(matrix3D));
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
			for each (IXbimShell^ shell in shells)
			{
				if (!shell->IsPolyhedron) return false;
			}
			return true;
		}


		IXbimGeometryObjectSet^ XbimShellSet::Cut(IXbimSolidSet^ solids, double tolerance)
		{
			
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance);
		}

		
		IXbimGeometryObjectSet^ XbimShellSet::Cut(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance);
		}

		IXbimGeometryObjectSet^ XbimShellSet::Union(IXbimSolidSet^ solids, double tolerance)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance);
		}

		void XbimShellSet::Union(double tolerance)
		{
		}

		IXbimGeometryObject ^ XbimShellSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimShellSet^ result = gcnew XbimShellSet();
			for each (XbimShell^ shell in shells)
				result->Add((XbimShell^)shell->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimShellSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimShellSet^ result = gcnew XbimShellSet();
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimShell^ shell in shells)
			{
				XbimShell^ copy = gcnew XbimShell((XbimShell^)shell, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimShellSet::Moved(IIfcObjectPlacement ^ objectPlacement)
		{
			if (!IsValid) return this;
			XbimShellSet^ result = gcnew XbimShellSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement);
			for each (IXbimShell^ shell in shells)
			{
				XbimShell^ copy = gcnew XbimShell((XbimShell^)shell, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimShellSet::Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimShell^ shell  in shells)
			{
				((XbimShell^)shell)->Mesh(mesh, precision, deflection, angle);
			}
		}

		IXbimGeometryObjectSet^ XbimShellSet::Union(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance);
		}

		IXbimGeometryObjectSet^ XbimShellSet::Intersection(IXbimSolidSet^ solids, double tolerance)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, (IEnumerable<IXbimGeometryObject^>^)this, solids, tolerance);
		}


		IXbimGeometryObjectSet^ XbimShellSet::Intersection(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimGeometryObjectSet::Empty;
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, (IEnumerable<IXbimGeometryObject^>^)this, gcnew XbimSolidSet(solid), tolerance);
		}
	}
}
