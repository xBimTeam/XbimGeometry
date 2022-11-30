#pragma once
#include "XShape.h"
#include <TopoDS_Compound.hxx>
#include <TopoDS.hxx>

using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;
#define OccCompound() TopoDS::Compound(*(this->Ptr()))
#define TOPO_COMPOUND(compound) TopoDS::Compound(static_cast<XbimCompound^>(compound)->GetTopoShape())
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XCompound : public XShape, IXCompound
			{
			public:
				XCompound(const TopoDS_Compound& hCompound) : XShape(new TopoDS_Compound(hCompound)) { };

				virtual property XShapeType ShapeType { XShapeType get() override { return XShapeType::Compound; } };

				virtual property bool IsCompoundsOnly {bool get(); };
				virtual property bool IsSolidsOnly {bool get(); };
				virtual property bool IsShellsOnly {bool get(); };
				virtual property bool IsFacesOnly {bool get(); };
				virtual property bool IsWiresOnly {bool get(); };
				virtual property bool IsEdgesOnly {bool get(); };
				virtual property bool IsVerticesOnly {bool get(); };
				virtual property bool HasCompounds {bool get(); };
				virtual property bool HasSolids {bool get(); };
				virtual property bool HasShells {bool get(); };
				virtual property bool HasFaces {bool get(); };
				virtual property bool HasWires {bool get(); };
				virtual property bool HasEdges {bool get(); };
				virtual property bool HasVertices {bool get(); };
				virtual property array<IXCompound^>^ Compounds { array<IXCompound^>^ get(); };
				virtual property array<IXSolid^>^ Solids { array<IXSolid^>^ get(); };
				virtual property array<IXShell^>^ Shells { array<IXShell^>^ get(); };
				virtual property array<IXFace^>^ Faces {  array<IXFace^>^ get(); };
				virtual property array<IXWire^>^ Wires {  array<IXWire^>^ get(); };
				virtual property array<IXEdge^>^ Edges {  array<IXEdge^>^ get(); };
				virtual property array<IXVertex^>^ Vertices {  array<IXVertex^ >^ get(); };
				virtual void Add(IXShape^ shape);

			};
		}
	}
}

