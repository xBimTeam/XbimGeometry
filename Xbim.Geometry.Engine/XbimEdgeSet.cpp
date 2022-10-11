
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TopExp.hxx>
#include "XbimEdgeSet.h"
#include "XbimConvert.h"
#include <BRep_Builder.hxx>
namespace Xbim
{ 
	namespace Geometry
	{
		XbimEdgeSet::operator  TopoDS_Shape ()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimEdge ^ edge in edges)
			{
				builder.Add(bodyCompound, (XbimEdgeV5^)edge);
			}
			return bodyCompound;
		}
		XbimEdgeSet::XbimEdgeSet(IEnumerable<IXbimEdge^>^ edges)
		{
			this->edges = gcnew List<IXbimEdge^>(edges);
		}

		XbimEdgeSet::XbimEdgeSet(XbimWireV5^ wire)
		{
			edges = gcnew  List<IXbimEdge^>();
			for (BRepTools_WireExplorer wireEx((const TopoDS_Wire&)wire); wireEx.More(); wireEx.Next())
			{
				edges->Add(gcnew XbimEdgeV5(wireEx.Current()));
			}
			System::GC::KeepAlive(wire);
		}

		XbimEdgeSet::XbimEdgeSet(const TopoDS_Shape& shape)
		{
			if (shape.ShapeType() == TopAbs_WIRE) //consider the ordering of the edges in the wire 
			{
				edges = gcnew  List<IXbimEdge^>();
				for (BRepTools_WireExplorer wireEx(TopoDS::Wire(shape)); wireEx.More(); wireEx.Next())
				{
					//bool reversed = (wireEx.Orientation() == TopAbs_REVERSED);
					//if (reversed)
					//	edges->Add(gcnew XbimEdge(TopoDS::Edge(wireEx.Current().Reversed())));
					//else
						edges->Add(gcnew XbimEdgeV5(wireEx.Current()));
				}
			}
			else
			{
				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(shape, TopAbs_EDGE, map);
				edges = gcnew  List<IXbimEdge^>(map.Extent());
				for (int i = 1; i <= map.Extent(); i++)
					edges->Add(gcnew XbimEdgeV5(TopoDS::Edge(map(i))));
			}

		}

		IXbimEdge^ XbimEdgeSet::First::get()
		{
			if (edges->Count==0) return nullptr;
			return edges[0];
		}

		int XbimEdgeSet::Count::get()
		{	
			return edges == nullptr ? 0 : edges->Count;
		}

		XbimRect3D XbimEdgeSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in edges)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IXbimGeometryObject^ XbimEdgeSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimEdge^>^ result = gcnew List<IXbimEdge^>(edges->Count);
			for each (IXbimGeometryObject^ edge in edges)
			{
				result->Add((IXbimEdge^)edge->Transform(matrix3D));
			}
			return gcnew XbimEdgeSet(result);
		}

		IXbimGeometryObject^ XbimEdgeSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimEdge^>^ result = gcnew List<IXbimEdge^>(edges->Count);
			for each (IXbimGeometryObject^ edge in edges)
			{
				result->Add((IXbimEdge^)((XbimEdgeV5^)edge)->TransformShallow(matrix3D));
			}
			return gcnew XbimEdgeSet(result);
		}

		IXbimGeometryObject ^ XbimEdgeSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimEdgeSet^ result = gcnew XbimEdgeSet();
			
			for each (XbimEdgeV5^ edge in edges)
				result->Add((XbimEdgeV5^)edge->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimEdgeSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimEdgeSet^ result = gcnew XbimEdgeSet();
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimEdge^ edge in edges)
			{
				XbimEdgeV5^ copy = gcnew XbimEdgeV5((XbimEdgeV5^)edge, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimEdgeSet::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimEdgeSet^ result = gcnew XbimEdgeSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement,logger);
			for each (IXbimEdge^ edge in edges)
			{
				XbimEdgeV5^ copy = gcnew XbimEdgeV5((XbimEdgeV5^)edge, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimEdgeSet::Mesh(IXbimMeshReceiver ^ /*mesh*/, double /*precision*/, double /*deflection*/, double /*angle*/)
		{
			return;//maybe add an implementation for this
		}

		IEnumerator<IXbimEdge^>^ XbimEdgeSet::GetEnumerator()
		{
			return edges->GetEnumerator();
		}
	}
}
