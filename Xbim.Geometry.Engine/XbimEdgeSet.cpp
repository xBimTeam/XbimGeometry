#include "XbimEdgeSet.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TopExp.hxx>
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		XbimEdgeSet::XbimEdgeSet(IEnumerable<IXbimEdge^>^ edges)
		{
			this->edges = gcnew List<IXbimEdge^>(edges);
		}

		XbimEdgeSet::XbimEdgeSet(XbimWire^ wire)
		{
			edges = gcnew  List<IXbimEdge^>();
			for (BRepTools_WireExplorer wireEx((const TopoDS_Wire&)wire); wireEx.More(); wireEx.Next())
			{
				edges->Add(gcnew XbimEdge(wireEx.Current()));
			}
			GC::KeepAlive(wire);
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
						edges->Add(gcnew XbimEdge(wireEx.Current()));
				}
			}
			else
			{
				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(shape, TopAbs_EDGE, map);
				edges = gcnew  List<IXbimEdge^>(map.Extent());
				for (int i = 1; i <= map.Extent(); i++)
					edges->Add(gcnew XbimEdge(TopoDS::Edge(map(i))));
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


		IEnumerator<IXbimEdge^>^ XbimEdgeSet::GetEnumerator()
		{
			return edges->GetEnumerator();
		}
	}
}
