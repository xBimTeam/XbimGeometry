#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Vertex.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimVertex :XbimHandle<TopoDS_Vertex>, IXVertex
			{

			public:
				XbimVertex(const TopoDS_Vertex& hVertex) : XbimHandle(new TopoDS_Vertex(hVertex)) {};
				virtual property XShapeType ShapeType {XShapeType get() { return XShapeType::Vertex; }};
				virtual property double Tolerance { double get(); };
				virtual property IXPoint^ VertexGeometry {IXPoint^ get(); };
			};
		}
	}
}
