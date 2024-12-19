#pragma once
#include "XShape.h"
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
using namespace Xbim::Geometry::Abstractions;
#define OccEdge() TopoDS::Edge(*(this->Ptr()))
#define TOPO_EDGE(edge) TopoDS::Edge(static_cast<XbimEdge^>(edge)->GetTopoShape())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			
			public ref class XEdge : public XShape, IXEdge
			{
			public:
				XEdge(const TopoDS_Edge& hEdge) : XShape(new TopoDS_Edge(hEdge)) {  };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Edge; } };
				virtual property double Tolerance {double get(); };
				virtual property IXVertex^ EdgeStart {IXVertex^ get(); };
				virtual property IXVertex^ EdgeEnd {IXVertex^ get(); };
				virtual property IXCurve^ EdgeGeometry {IXCurve^ get(); };
				virtual property double Length {double get(); }

				
			};
		}
	}
}

