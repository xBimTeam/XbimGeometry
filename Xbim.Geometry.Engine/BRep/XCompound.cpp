#include "XCompound.h"
#include "XSolid.h"
#include "XShell.h"
#include "XFace.h"
#include "XWire.h"
#include "XEdge.h"
#include "XVertex.h"
//#include "XShape.h"
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

			array<IXCompound^>^ XCompound::Compounds::get()
			{
				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_COMPOUND)
						shapes.Append(iter.Value());
				}
				array<IXCompound^>^ managedShapes = gcnew array<IXCompound^>(shapes.Size());
				int i = 0;
				for(auto&& shape: shapes)
					managedShapes[i++] = (gcnew XCompound(TopoDS::Compound(shape)));
				return managedShapes;
			};

			array<IXSolid^>^ XCompound::Solids::get()
			{
				
				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SOLID)
						shapes.Append(iter.Value());
				}
				array<IXSolid^>^ managedShapes = gcnew array<IXSolid^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = (gcnew XSolid(TopoDS::Solid(shape)));
				return managedShapes;
			};

			array<IXShell^>^ XCompound::Shells::get()
			{

				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_SHELL)
						shapes.Append(iter.Value());
				}
				array<IXShell^>^ managedShapes = gcnew array<IXShell^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = (gcnew XShell(TopoDS::Shell(shape)));
				return managedShapes;
			};

			array<IXFace^>^ XCompound::Faces::get()
			{
				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_FACE)
						shapes.Append(iter.Value());
				}
				array<IXFace^>^ managedShapes = gcnew array<IXFace^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = (gcnew XFace(TopoDS::Face(shape)));
				return managedShapes;
			};
			

			array<IXWire^>^ XCompound::Wires::get()
			{
				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_WIRE)
						shapes.Append(iter.Value());
				}
				array<IXWire^>^ managedShapes = gcnew array<IXWire^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = (gcnew XWire(TopoDS::Wire(shape)));
				return managedShapes;
			};


			array<IXEdge^>^ XCompound::Edges::get()
			{
				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_EDGE)
						shapes.Append(iter.Value());
				}
				array<IXEdge^>^ managedShapes = gcnew array<IXEdge^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = (gcnew XEdge(TopoDS::Edge(shape)));
				return managedShapes;
			};

			array<IXVertex^>^ XCompound::Vertices::get()
			{
				TopoDS_ListOfShape shapes;
				for (TopoDS_Iterator iter(OccCompound()); iter.More(); iter.Next())
				{
					if (iter.Value().ShapeType() == TopAbs_VERTEX)
						shapes.Append(iter.Value());
				}
				array<IXVertex^>^ managedShapes = gcnew array<IXVertex^>(shapes.Size());
				int i = 0;
				for (auto&& shape : shapes)
					managedShapes[i++] = (gcnew XVertex(TopoDS::Vertex(shape)));
				return managedShapes;
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