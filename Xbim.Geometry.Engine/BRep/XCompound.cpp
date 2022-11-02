#include "XCompound.h"
#include "XSolid.h"
#include "XShell.h"
#include "XFace.h"
#include "XWire.h"
#include "XEdge.h"
#include "XVertex.h"
#include "XShape.h"
#include <TopoDS_Iterator.hxx>
#include <TopoDS.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <GProp_PGProps.hxx>
#include <BRepGProp.hxx>
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			
			bool XCompound::IsCompoundsOnly::get()
			{
				bool isCompoundsOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_COMPOUND)
						return false;
					else
						isCompoundsOnly = true; //we have a solid
				};
				return isCompoundsOnly;
			};

			bool XCompound::IsSolidsOnly::get()
			{				
				bool isSolidsOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_SOLID)
						return false;
					else
						isSolidsOnly = true; //we have a solid
				};
				return isSolidsOnly;
			};


			bool XCompound::IsShellsOnly::get()
			{
				
				bool isShellsOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_SHELL)
						return false;
					else
						isShellsOnly = true; //we have a shell					
				};
				return isShellsOnly;
			};
			bool XCompound::IsFacesOnly::get()
			{
				
				bool isFacesOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_FACE)
						return false;
					else
						isFacesOnly = true; //we have a face					
				};
				return isFacesOnly;
			};

			bool XCompound::IsWiresOnly::get()
			{

				bool isWiresOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_WIRE)
						return false;
					else
						isWiresOnly = true; //we have a wire					
				};
				return isWiresOnly;
			};

			bool XCompound::IsEdgesOnly::get()
			{

				bool isEdgesOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_EDGE)
						return false;
					else
						isEdgesOnly = true; //we have an edge					
				};
				return isEdgesOnly;
			};

			bool XCompound::IsVerticesOnly::get()
			{
				bool isVerticesOnly = false;
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() != TopAbs_VERTEX)
						return false;
					else
						isVerticesOnly = true; //we have an vertex					
				};
				return isVerticesOnly;
			};

			bool XCompound::HasCompounds::get()
			{
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_COMPOUND) return true;
				}
				return false;
			};
			bool XCompound::HasSolids::get()
			{
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SOLID) return true;					
				}
				return false;
			};

			bool XCompound::HasShells::get()
			{
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SHELL) return true;					
				}
				return false;
			};

			bool XCompound::HasFaces::get()
			{
				for (TopoDS_Iterator iter(OccCompound(),false,false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_FACE) return true;					
				}
				return false;
			};

			bool XCompound::HasWires::get()
			{
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_WIRE) return true;
				}
				return false;
			};
			
			bool XCompound::HasEdges::get()
			{
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_EDGE) return true;
				}
				return false;
			};

			bool XCompound::HasVertices::get()
			{
				for (TopoDS_Iterator iter(OccCompound(), false, false); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_VERTEX) return true;
				}
				return false;
			};

			IEnumerable<IXCompound^>^ XCompound::Compounds::get()
			{

				List<IXCompound^>^ compounds = gcnew List<IXCompound^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_COMPOUND)
						compounds->Add(gcnew XCompound(TopoDS::Compound(iter.Value())));
				}
				return compounds;
			};

			IEnumerable<IXSolid^>^ XCompound::Solids::get()
			{
				
				List<IXSolid^>^ solids = gcnew List<IXSolid^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SOLID)
						solids->Add(gcnew XSolid(TopoDS::Solid(iter.Value())));
				}
				return solids;
			};

			IEnumerable<IXShell^>^ XCompound::Shells::get()
			{

				List<IXShell^>^ shells = gcnew List<IXShell^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SHELL)
						shells->Add(gcnew XShell(TopoDS::Shell(iter.Value())));
				}
				return shells;
			};

			IEnumerable<IXFace^>^ XCompound::Faces::get()
			{
				List<IXFace^>^ faces = gcnew List<IXFace^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_FACE) faces->Add(gcnew XFace(TopoDS::Face(iter.Value())));
				}
				return faces;
			};

			IEnumerable<IXWire^>^ XCompound::Wires::get()
			{
				List<IXWire^>^ wires = gcnew List<IXWire^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_WIRE)
						wires->Add(gcnew XWire(TopoDS::Wire(iter.Value())));
				}
				return wires;
			};


			IEnumerable<IXEdge^>^ XCompound::Edges::get()
			{
				List<IXEdge^>^ edges = gcnew List<IXEdge^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_EDGE) 
						edges->Add(gcnew XEdge(TopoDS::Edge(iter.Value())));
				}
				return edges;
			};

			IEnumerable<IXVertex^>^ XCompound::Vertices::get()
			{
				List<IXVertex^>^ vertices = gcnew List<IXVertex^>();
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_EDGE)
						vertices->Add(gcnew XVertex(TopoDS::Vertex(iter.Value())));
				}
				return vertices;
			};

			void XCompound::Add(IXShape^ shape)
			{
				BRep_Builder builder;
				const TopoDS_Shape& topoShape = static_cast<XShape^>(shape)->GetTopoShape();
				builder.Add(Ref(), topoShape);
			}
		}
	}
}