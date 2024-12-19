#pragma once
#include "XShape.h"
#include <TopoDS_Vertex.hxx>
#include <TopoDS.hxx>
#define OccVertex() TopoDS::Vertex(*(this->Ptr()))
#define TOPO_VERTEX(vertex) TopoDS::Vertex(static_cast<XbimVertex^>(vertex)->GetTopoShape())
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XVertex :public XShape, IXVertex
			{

			public:
				XVertex(const TopoDS_Vertex& hVertex) : XShape(new TopoDS_Vertex(hVertex)) {};
				virtual property XShapeType ShapeType {XShapeType get() override { return XShapeType::Vertex; }};
				virtual property double Tolerance { double get(); };
				virtual property IXPoint^ VertexGeometry {IXPoint^ get(); };

				
			};
		}
	}
}
