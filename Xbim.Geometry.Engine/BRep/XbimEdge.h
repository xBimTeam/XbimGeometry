#pragma once
#include "XbimShape.h"
#include <TopoDS_Edge.hxx>
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XbimEdge : public XbimShape<TopoDS_Edge>, IXEdge
			{
			public:
				XbimEdge(const TopoDS_Edge& hEdge) : XbimShape(new TopoDS_Edge(hEdge)) {  };
				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Edge; } };
				virtual property double Tolerance {double get(); };
				virtual property IXVertex^ EdgeStart {IXVertex^ get(); };
				virtual property IXVertex^ EdgeEnd {IXVertex^ get(); };
				virtual property IXCurve^ EdgeGeometry {IXCurve^ get(); };
			};
		}
	}
}

